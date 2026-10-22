# SUBJILLA — Smart Home Automation

SUBJILLA is an Arduino UNO smart-home automation project with automatic street lighting, water-level-based pump control, gas detection, a window servo, and a sensor-controlled lift.

> **Note:** There is no roof button or roof servo in this version.

## Components

- Arduino UNO
- LDR module
- MQ-2 gas sensor module
- Flat PCB water-level sensor
- Lift sensor (digital output)
- L298N motor driver
- Water pump motor
- Fan motor
- Window servo
- Lift servo
- 3 LEDs: street light, gas warning, and tank-full indicator
- Suitable power supplies, jumper wires, and LED resistors

## Complete Wiring

| No. | Component / Pin | Connect to |
|---:|---|---|
| 1 | LDR module AO / Signal | Arduino UNO A0 |
| 2 | LDR module VCC | Arduino 5V, if supported by the module |
| 3 | LDR module GND | Arduino GND |
| 4 | MQ-2 gas sensor AO | Arduino A1 |
| 5 | MQ-2 VCC | Supply specified for the module |
| 6 | MQ-2 GND | Arduino GND |
| 7 | Water-level sensor S / SIG | Arduino A2 |
| 8 | Water-level sensor + / VCC | Arduino 5V, if supported by the sensor |
| 9 | Water-level sensor − / GND | Arduino GND |
| 10 | Lift sensor OUT | Arduino D12 |
| 11 | Lift sensor VCC | Supply specified for the sensor |
| 12 | Lift sensor GND | Arduino GND |
| 13 | L298N IN1 | Arduino D4 |
| 14 | L298N IN2 | Arduino D7 |
| 15 | L298N IN3 | Arduino D6 |
| 16 | L298N IN4 | Arduino D8 |
| 17 | L298N ENA jumper | Leave installed |
| 18 | L298N ENB jumper | Leave installed |
| 19 | L298N OUT1 and OUT2 | Water pump motor |
| 20 | L298N OUT3 and OUT4 | Fan motor |
| 21 | L298N motor supply | Suitable supply for the motors |
| 22 | L298N GND | Common ground |
| 23 | Street-light LED anode (+) | Arduino D5 through a 220–330 Ω resistor |
| 24 | Street-light LED cathode (−) | GND |
| 25 | Gas-warning LED anode (+) | Arduino A3 through a 220–330 Ω resistor |
| 26 | Gas-warning LED cathode (−) | GND |
| 27 | Tank-full LED anode (+) | Arduino A4 through a 220–330 Ω resistor |
| 28 | Tank-full LED cathode (−) | GND |
| 29 | Window servo signal | Arduino D9 |
| 30 | Window servo VCC | Suitable regulated servo supply |
| 31 | Window servo GND | Common ground |
| 32 | Lift servo signal | Arduino D11 |
| 33 | Lift servo VCC | Suitable regulated servo supply |
| 34 | Lift servo GND | Common ground |
| 35 | Common ground | Join Arduino GND, L298N GND, sensor GNDs, and external supply negatives |
| 36 | Arduino D2 and D3 | Unused; old tank switches removed |
| 37 | Roof button / roof servo | Not used |

## System Behaviour

1. **Automatic street light:** Adjusts LED brightness based on the LDR reading.
2. **Automatic water tank:** Starts the pump when the measured water level reaches the configured low threshold and stops it at the configured full threshold. The tank-full LED turns on at the full threshold.
3. **Gas detection:** When the MQ-2 reading exceeds its configured threshold, the warning LED and fan turn on and the window servo opens. When the reading falls below the reset threshold, the fan turns off and the window servo closes.
4. **Lift:** Moves toward the configured up position while the lift sensor output is HIGH and returns down when it is LOW.

## Arduino Code

The Arduino sketch should use these pin assignments:

```cpp
const int LDR_PIN = A0;
const int GAS_PIN = A1;
const int WATER_LEVEL_PIN = A2;
const int LIFT_SENSOR_PIN = 12;

const int PUMP_IN1 = 4;
const int STREET_LED = 5;
const int FAN_IN1 = 6;
const int PUMP_IN2 = 7;
const int FAN_IN2 = 8;

const int WINDOW_PIN = 9;
const int LIFT_PIN = 11;

const int GAS_LED = A3;
const int TANK_FULL_LED = A4;
```

The full sketch is maintained separately as the Arduino `.ino` file. The example water thresholds in the sketch must be calibrated for the actual sensor before operating the pump.

## Calibration and Testing

1. Disconnect motor power before uploading and testing the sketch.
2. Open the Arduino IDE Serial Monitor and select **9600 baud**.
3. Observe the water sensor's readings when dry and at several known water levels. Keep the sensor's electronic components dry.
4. Set the low and full thresholds to values measured from your sensor. Do not assume example values will work for every module.
5. Test the pump-off behaviour and sensor wiring before running the pump. A disconnected sensor can produce misleading readings; do not leave the pump unattended until fault handling has been tested.
6. Check the lift sensor's output polarity. If it is active LOW, the lift logic must be inverted.
7. Test the servos without mechanical loads first.

## Power and Safety

- Do not power the pump, fan, or servos directly from Arduino GPIO pins.
- Use a suitable motor supply for the pump and fan, and a regulated supply suitable for the servos.
- Connect all circuit grounds together.
- Verify the L298N board's motor-supply and logic-power jumper arrangement before applying power. Do not assume every L298N board is wired identically.
- Confirm the voltage requirements and pin labels of every sensor before connecting it.
- Use the project only as a supervised demonstration. An MQ-2 sensor and hobby fan/window setup are **not** a certified gas detection or emergency ventilation system. Do not rely on this project for real-world gas safety.

## Pin Summary

| Arduino pin | Function |
|---|---|
| A0 | LDR signal |
| A1 | MQ-2 analog output |
| A2 | Water-level sensor signal |
| A3 | Gas-warning LED |
| A4 | Tank-full LED |
| D2, D3 | Unused |
| D4 | Pump driver IN1 |
| D5 | Street-light LED |
| D6 | Fan driver IN3 |
| D7 | Pump driver IN2 |
| D8 | Fan driver IN4 |
| D9 | Window servo signal |
| D11 | Lift servo signal |
| D12 | Lift sensor output |

---
**Project:** SUBJILLA Smart Home Automation  
**Controller:** Arduino UNO  
**Roof feature:** Removed
