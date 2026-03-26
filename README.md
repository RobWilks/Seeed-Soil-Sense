# Seeed-Soil-Sense

Code and watchdog circuit to integrate Seeed industrial soil sensor with Home Assistant:

https://github.com/RobWilks/Seeed-Soil-Sense/blob/main/Soil%20Moisture%26Temperature%26EC%20Sensor%20User%20Manual-S-Temp%26VWC%26EC-02.pdf

The soil sensor connects to a Lilygo T-CAN board via RS485 and Modbus protocol:

https://github.com/Xinyuan-LilyGO/T-CAN485/blob/arduino-esp32-libs_v3.0.1/README.md

The board has a ESP32 microcontroller to manage the bus and transmit soil moisture, conductivity and temperature data via Bluetooth advertising packets
The data are measured at a low frequency of 10 minutes with the board and sensor powered off between measurements.
Power management is by a separate watchdog circuit using a ATTiny85 microcontroller and P-channel MOSFET switch:

https://github.com/RobWilks/Seeed-Soil-Sense/blob/main/TCAN_Soil_Sense_circuit.png

built on a small stripboard breakout:

https://github.com/RobWilks/Seeed-Soil-Sense/blob/main/tinyWatchdogVoltageSwitchV3.pdf

and powered using 2x 18650 lithium ion batteries in series (~1500 mAh).

The Arduino framework was used to develop the firmware for the board and watchdog timer.  The project libraries are in separate directories:

https://github.com/RobWilks/Seeed-Soil-Sense/tree/main/esp32_BLE_Soil_SenseV4

https://github.com/RobWilks/Seeed-Soil-Sense/tree/main/ATTiny85_Watchdog_Soil_SenseV2

and compiled using Arduino IDE:

https://www.arduino.cc/en/software/

=============================================

CIRCUIT PERFORMANCE

Standby current: ~90 microA

Average current when board and sensor powered on: 37 mA

Note peak current when transmitting 100-200 mA, sensor current draw 6.3 mA

Duration of on cycle: 3900 msec

Estimated battery lifetime: 186 d


