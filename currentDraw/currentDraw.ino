// Measure discharge curve of lithium ion battery
// Battery connected to ground via INA219 current sense, 
// load resistor and FQP27POS pnp MOSFET power switch
// G of MOSFET connected to collector of npn bc547 via 1k resistor
// G pulled to supply rail via 47k resistor
// base of BC547 connected to D3 of UNO via 10k resistor
// Discharge and logging stopped by D1 low when current falls below 101mA = 6V / Rload


#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

// --- CONFIGURATION ---
const int CONTROL_PIN = 3;     // WARNING: Pin 1 is Serial TX. Consider using D3.
const float CUTOFF_CURRENT = 101; // mA (6V / 82R||220R)
const float SAFE_VOLTAGE_LIMIT = 6.0; // Volts (Safe 2S Li-ion cutoff)
bool isDischarging = true;


void setup(void) {
  // Initialize Control Pin immediately to prevent accidental discharge
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, HIGH); // Enable discharge via BC547/P-MOS

  Serial.begin(115200);
  while (!Serial) { delay(1); } 

  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    digitalWrite(CONTROL_PIN, LOW); // Safety shutoff if sensor is missing
    while (1) { delay(10); }
  }

  // 16V_400mA provides 0.1mA resolution, perfect for 44mA-100mA range.
  ina219.setCalibration_16V_400mA();

  Serial.println("Status: DISCHARGE ENABLED");
  Serial.println("Timestamp(ms),Voltage(V),Current(mA)");
}


void loop(void) {
  if (!isDischarging) return; // Stop everything if discharge is complete

  float current_mA = ina219.getCurrent_mA();
  float busVoltage = ina219.getBusVoltage_V(); 

  // 1. Data Logging (CSV Format) 
  if (current_mA > 0) {
    Serial.print(millis());
    Serial.print(",");
    Serial.print(busVoltage);
    Serial.print(",");
    Serial.println(current_mA);
  }  

// 2. Safety Logic: Stop if Current OR Voltage drops too low
  bool lowCurrent = (current_mA < CUTOFF_CURRENT);
  bool lowVoltage = (busVoltage < SAFE_VOLTAGE_LIMIT);

  if ((lowCurrent || lowVoltage) && millis() > 2000) { 
    digitalWrite(CONTROL_PIN, LOW); // Cut off the BC547 base 
    isDischarging = false;
    
    Serial.print("--- SHUTDOWN: ");
    if (lowVoltage) Serial.print("LOW VOLTAGE ");
    if (lowCurrent) Serial.print("LOW CURRENT ");
    Serial.println("---");
  }

  delay(20000); // 20s sampling
}
