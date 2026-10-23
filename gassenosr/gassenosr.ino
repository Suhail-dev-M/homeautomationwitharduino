
#include <Arduino.h>
#include <Servo.h>

// Gas sensor
constexpr uint8_t GAS_SENSOR_PIN = A0;
constexpr int GAS_THRESHOLD = 230;

// Rain sensor
constexpr uint8_t RAIN_SENSOR_PIN = 5;  // D1 / GPIO5

// Separate servo motors
constexpr uint8_t GAS_SERVO_PIN = 14;   // D5 / GPIO14
constexpr uint8_t RAIN_SERVO_PIN = 4;   // D2 / GPIO4

// Gas warning LED
constexpr uint8_t WARNING_LED_PIN = 12; // D6 / GPIO12

// Servo positions
constexpr int WINDOW_CLOSED_ANGLE = 0;
constexpr int GAS_WINDOW_OPEN_ANGLE = 90;
constexpr int RAIN_SERVO_ANGLE = 180;

// Most digital rain sensor modules output LOW when wet.
constexpr uint8_t RAIN_DETECTED_LEVEL = LOW;

Servo gasServo;
Servo rainServo;

bool gasWindowOpen = false;
bool rainServoActivated = false;

void setup() {
  Serial.begin(115200);

  pinMode(RAIN_SENSOR_PIN, INPUT);
  pinMode(WARNING_LED_PIN, OUTPUT);

  digitalWrite(WARNING_LED_PIN, LOW);

  gasServo.attach(GAS_SERVO_PIN);
  rainServo.attach(RAIN_SERVO_PIN);

  gasServo.write(WINDOW_CLOSED_ANGLE);
  rainServo.write(0);

  Serial.println();
  Serial.println(F("ESP8266 Gas and Rain Detection System"));
  Serial.print(F("Gas threshold: "));
  Serial.println(GAS_THRESHOLD);
  Serial.println(F("System ready."));
}

void loop() {
  // Read gas sensor
  int gasValue = analogRead(GAS_SENSOR_PIN);
  bool gasDetected = (gasValue >= GAS_THRESHOLD);

  // Read rain sensor
  bool rainDetected =
      (digitalRead(RAIN_SENSOR_PIN) == RAIN_DETECTED_LEVEL);

  // GAS CONTROL: Servo 1 and warning LED
  digitalWrite(WARNING_LED_PIN, gasDetected ? HIGH : LOW);

  if (gasDetected && !gasWindowOpen) {
    gasServo.write(GAS_WINDOW_OPEN_ANGLE);
    gasWindowOpen = true;
    Serial.println(F("GAS DETECTED: Gas servo opening window."));
  } else if (!gasDetected && gasWindowOpen) {
    gasServo.write(WINDOW_CLOSED_ANGLE);
    gasWindowOpen = false;
    Serial.println(F("Gas normal: Gas servo closing window."));
  }

  // RAIN CONTROL: Servo 2
  if (rainDetected && !rainServoActivated) {
    rainServo.write(RAIN_SERVO_ANGLE);
    rainServoActivated = true;
    Serial.println(F("RAIN DETECTED: Rain servo moving to 180 degrees."));
  } else if (!rainDetected && rainServoActivated) {
    rainServo.write(0);
    rainServoActivated = false;
    Serial.println(F("Rain stopped: Rain servo returning to 0 degrees."));
  }

  // Serial Monitor status
  Serial.print(F("Gas: "));
  Serial.print(gasValue);
  Serial.print(F(" | Threshold: "));
  Serial.print(GAS_THRESHOLD);
  Serial.print(F(" | Gas: "));
  Serial.print(gasDetected ? F("DETECTED") : F("NORMAL"));
  Serial.print(F(" | Rain: "));
  Serial.print(rainDetected ? F("DETECTED") : F("NONE"));
  Serial.print(F(" | Gas servo: "));
  Serial.print(gasWindowOpen ? 90 : 0);
  Serial.print(F(" | Rain servo: "));
  Serial.println(rainServoActivated ? 180 : 0);

  delay(300);
}
