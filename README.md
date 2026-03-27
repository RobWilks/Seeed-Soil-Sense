# Seeed-Soil-Sense

Code and watchdog circuit to integrate [Seeed industrial soil sensor](https://files.seeedstudio.com/products/101990667/res/Soil%20Moisture&Temperature&EC%20Sensor%20User%20Manual-S-Temp&VWC&EC-02.pdf) with Home Assistant:

The soil sensor connects to a [Lilygo T-CAN485 board](https://github.com/Xinyuan-LilyGO/T-CAN485/blob/arduino-esp32-libs_v3.0.1/README.md) via RS485 and Modbus protocol:

The board has a ESP32 microcontroller to manage the bus and transmit soil moisture, conductivity and temperature data via Bluetooth advertising packets
The data are measured at a low frequency of 10 minutes with the board and sensor powered off between measurements.
Power management is by a separate [watchdog circuit](TCAN_Soil_Sense_circuit.png) using a [ATTiny85 microcontroller](https://ww1.microchip.com/downloads/en/devicedoc/atmel-2586-avr-8-bit-microcontroller-attiny25-attiny45-attiny85_datasheet.pdf) and P-channel MOSFET switch:
built on a small [stripboard breakout](tinyWatchdogVoltageSwitchV3.png) and powered using 2x 18650 lithium ion batteries in series (~1500 mAh).

The Arduino framework was used to develop the [firmware for the board](esp32_BLE_Soil_SenseV4/esp32_BLE_Soil_SenseV4.ino) and [firmware for the watchdog timer](ATTiny85_Watchdog_Soil_SenseV2/ATTiny85_Watchdog_Soil_SenseV2.ino).  The project libraries are in separate directories and compiled using [Arduino IDE](https://www.arduino.cc/en/software/).  Bluetooth advertising uses the [BTHome library](https://www.espruino.com/BTHome).  A simple packet counter is used to keep track of any packet loss.  Loss can be minimised by siting a [espHome BTProxy](https://esphome.io/components/bluetooth_proxy/) gateway near to the Lilygo board, which provides a bridge between bluetooth and the WiFi LAN.



=============================================

## CIRCUIT PERFORMANCE

Standby current: ~90 microA

Average current when board and sensor powered on: 37 mA

Peak current when transmitting: 100-200 mA, sensor current: 6.3 mA

Duration of on cycle: 3900 msec

Estimated battery lifetime: 186 d


=============================================

## INSTALLATION

Sensor buried approximately 15 cm deep (see photos).
