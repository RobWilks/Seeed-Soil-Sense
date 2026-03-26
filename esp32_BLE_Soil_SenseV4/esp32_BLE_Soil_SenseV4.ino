// firmware for lilycan 485 board that interfaces to Seeed Sensecap Model: S-Temp&VWC&EC-02
// The sensor reports moisture, conductivity and temperature
// Results are broadcast by BLE - in advertising mode
// Battery voltage is measured by esp32 ADC and also reported
// Lilycan powered via a AtTiny85 Watchdog.  The Lilycan signals to the Watchdog to powerdown at the end of each measurement
// 
// Measurement and advertising duration is about 3900ms; frequency is set by the watchdog at 144 measurements/day
// Average current for on period is 37mA

#include "BTHome.h"
#include <ModbusMaster.h>
// uses a portion of the on-board non-volatile memory (NVS) of the ESP32 to store packet count
#include <Preferences.h>
#include <NimBLEDevice.h>
#include "esp_mac.h"

// --- BATTERY VOLTAGE & SIGNAL CONFIG ---
#define PIN_ATTINY_V_REQ 18          // Input from ATtiny85 PB0
#define PIN_V_SENSE 32               // ADC1 Channel4 for battery voltage
#define VOLTAGE_DIVIDER_RATIO 3.127  // Calculated for 100k/47k resistors

// --- EXISTING HARDWARE CONFIG ---
#define PIN_SD_CS 13
#define PIN_5V_POWER 16
#define PIN_RS485_AUTODIR 17
#define PIN_RS485_ENABLE 19
#define PIN_CAN_STANDBY 23
#define PIN_MEASURE_DONE 25  // Output to ATTiny85 PB2
#define PIN_RS485_TX 22
#define PIN_RS485_RX 21

#define MODBUS_ADDRESS 0x01
#define DEVICE_NAME "SS"                             // keep name short to increase space in packet for data
#define BIND_KEY "7f8a2c4e91b0d5e638f2a1c9d4b7e052"  // Not used - BTHome configured for non-encrypted


BTHome bthome;
ModbusMaster node;

void setup() {
  // 1. Hardware Initialization
  pinMode(PIN_MEASURE_DONE, OUTPUT);
  digitalWrite(PIN_MEASURE_DONE, LOW);

  pinMode(PIN_ATTINY_V_REQ, INPUT);  // Request signal from ATtiny85
  // Standard INPUT mode (No pull-up, no pull-down)
  pinMode(PIN_V_SENSE, INPUT);
  analogSetAttenuation(ADC_11db);  // Set ADC range to ~3.6V

  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);
  pinMode(PIN_CAN_STANDBY, OUTPUT);
  digitalWrite(PIN_CAN_STANDBY, HIGH);
  pinMode(PIN_5V_POWER, OUTPUT);
  digitalWrite(PIN_5V_POWER, HIGH);
  pinMode(PIN_RS485_ENABLE, OUTPUT);
  digitalWrite(PIN_RS485_ENABLE, HIGH);
  pinMode(PIN_RS485_AUTODIR, OUTPUT);
  digitalWrite(PIN_RS485_AUTODIR, HIGH);

  // 2. Communication Setup
  Serial1.begin(9600, SERIAL_8N1, PIN_RS485_RX, PIN_RS485_TX);
  node.begin(MODBUS_ADDRESS, Serial1);

  // 1. Force a new Identity (removal of zombie entity caused by earlier device exceeding packet length)
  // Factory BT was 2C:BC:BB:A7:E1:36. We set Base to ...37
  uint8_t new_mac[] = {0x2C, 0xBC, 0xBB, 0xA7, 0xE1, 0x37}; 
  esp_base_mac_addr_set(new_mac);

  bthome.begin(DEVICE_NAME, false, BIND_KEY, false);
  // SET RF POWER HERE
  // Options: ESP_PWR_LVL_N12, N9, N6, N3, N0 (Default), P3, P6, P9 (Max)
  NimBLEDevice::setPower(ESP_PWR_LVL_P3);

  delay(2000);  // Sensor warm-up
}

void loop() {

  uint8_t result;
  float temp = 0, vwc = 0, voltage = 0;
  uint64_t ec = 0;
  bool requestVoltage = false;

  // Write packet count to NVS
  Preferences preferences;
  preferences.begin("bthome_store", false);

  // Retrieve last counter, increment it, and save it
  // Packet ID used to show packet loss
  // NVS preferences library has wear levelling to rotate location of read/writes 
  uint32_t packet_count = preferences.getUInt("counter", 0);
  packet_count++;
  preferences.putUInt("counter", packet_count);

  preferences.end();


  // 3. Check for Voltage Measurement Request
  if (digitalRead(PIN_ATTINY_V_REQ) == HIGH)  {
    requestVoltage = true;
    int rawADC = analogRead(PIN_V_SENSE);
    // Convert ADC to actual battery voltage (6-8V range)
    // voltage = (static_cast<float>(rawADC) / 4095.0f) * 3.9f * VOLTAGE_DIVIDER_RATIO;
    // use calibrated values V = raw * 3.57e-3 + 0.509 and convert to V
    voltage = static_cast<float>(rawADC) * 3.57e-3 + 0.509f;
  }

  // 4. Modbus Reading
  result = node.readHoldingRegisters(0x0000, 3);
  if (result == node.ku8MBSuccess) {
    // 1. Temperature: Cast to signed int16 first to handle sub-zero temps, then to float
    temp = static_cast<float>(static_cast<int16_t>(node.getResponseBuffer(0))) * 0.01f;

    // 2. VWC: Direct cast to float (it's always positive)
    vwc = static_cast<float>(node.getResponseBuffer(1)) * 0.01f;

    // 3. EC: Direct cast to uint64_t (standard integer for conductivity)
    ec = static_cast<uint64_t>(node.getResponseBuffer(2));

    // 5. Build BTHome Packet
    bthome.resetMeasurement();
    bthome.addMeasurement(ID_COUNT2, (uint64_t)packet_count);  // 5 bytes
    bthome.addMeasurement(ID_TEMPERATURE_PRECISE, temp);       // 0x02 3 bytes
    bthome.addMeasurement(ID_MOISTURE_PRECISE, vwc);           // 0x14 3 bytes
    bthome.addMeasurement(ID_CONDUCTIVITY, ec);      //0x56 3 bytes
 
    if (requestVoltage) {
         bthome.addMeasurement(ID_VOLTAGE, voltage);  // 3 bytes
   }

    bthome.buildPaket();
    bthome.start();
    delay(1500);  // Ensure advertisement is picked up
    bthome.stop();
  }

  // 6. Signal Power Cut
  digitalWrite(PIN_MEASURE_DONE, HIGH);
  delay(50);
  digitalWrite(PIN_MEASURE_DONE, LOW);

  // Fallback
  ESP.deepSleep(3600e6);
}