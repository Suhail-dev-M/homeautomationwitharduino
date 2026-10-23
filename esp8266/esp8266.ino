
#include <Arduino.h>
#include <Servo.h>

// ============== GAS SENSOR ==============
constexpr uint8_t GAS_SENSOR_PIN = A0;
constexpr int GAS_THRESHOLD = 400;
constexpr uint8_t WARNING_LED_PIN = 12;  // D6 / GPIO12

// ============== RAIN SENSOR ==============
constexpr uint8_t RAIN_SENSOR_PIN = 5;   // D1 / GPIO5
constexpr uint8_t RAIN_DETECTED_LEVEL = LOW;

// ============== SERVOS ==============
constexpr uint8_t GAS_SERVO_PIN = 14;    // D5 / GPIO14
constexpr uint8_t RAIN_SERVO_PIN = 4;    // D2 / GPIO4

constexpr int GAS_WINDOW_CLOSED = 0;
constexpr int GAS_WINDOW_OPEN = 90;
constexpr int RAIN_SERVO_HOME = 0;
constexpr int RAIN_SERVO_ACTIVE = 180;

Servo gasServo;
Servo rainServo;

bool gasWindowOpen = false;
bool rainServoActivated = false;

// ============== IR SENSOR ==============
constexpr uint8_t IR_SENSOR_PIN = 13;    // D7 / GPIO13
constexpr uint8_t IR_DETECTED_LEVEL = LOW;

// ============== L298N MOTOR ==============
// Channel B: connect motor to OUT3 and OUT4.
// Keep ENB enabled (jumper fitted, if your board has one).
constexpr uint8_t MOTOR_IN3 = 0;         // D3 / GPIO0
constexpr uint8_t MOTOR_IN4 = 2;         // D4 / GPIO2

// Independent lift timings
constexpr unsigned long LIFT_UP_TIME = 5000UL;       // 5 seconds
constexpr unsigned long LIFT_RETURN_TIME = 2000UL;   // 2 seconds

enum LiftState {
  LIFT_LANDED,
  LIFT_MOVING_UP,
  LIFT_AT_TOP,
  LIFT_RETURNING
};

LiftState liftState = LIFT_LANDED;
unsigned long liftStartTime = 0;

// ============== MOTOR CONTROL ==============
void liftUp() {
  digitalWrite(MOTOR_IN3, HIGH);
  digitalWrite(MOTOR_IN4, LOW);
}

void liftDown() {
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, HIGH);
}

void liftStop() {
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
}

// ============== LIFT LOGIC ==============
void updateLift(bool objectDetected) {
  unsigned long now = millis();

  if (objectDetected) {
    // Start upward movement from the bottom or interrupt a return.
    if (liftState == LIFT_LANDED ||
        liftState == LIFT_RETURNING) {
      liftState = LIFT_MOVING_UP;
      liftStartTime = now;
      liftUp();
      Serial.println(F("IR DETECTED: Lift moving UP."));
    }

    if (liftState == LIFT_MOVING_UP) {
      if (now - liftStartTime >= LIFT_UP_TIME) {
        liftStop();
        liftState = LIFT_AT_TOP;
        Serial.println(F("Upward time finished: lift STOPPED."));
      } else {
        liftUp();
      }
    } else {
      liftStop();
    }
  } else {
    // When IR detection stops, start the timed return.
    if (liftState == LIFT_MOVING_UP ||
        liftState == LIFT_AT_TOP) {
      liftState = LIFT_RETURNING;
      liftStartTime = now;
      liftDown();
      Serial.println(F("IR LOST: Lift returning DOWN."));
    }

    if (liftState == LIFT_RETURNING) {
      if (now - liftStartTime >= LIFT_RETURN_TIME) {
        liftStop();
        liftState = LIFT_LANDED;
        Serial.println(F("Return time finished: lift STOPPED."));
      } else {
        liftDown();
      }
    } else {
      liftStop();
    }
  }
}

// ============== SETUP ==============
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(WARNING_LED_PIN, OUTPUT);
  digitalWrite(WARNING_LED_PIN, LOW);

  pinMode(RAIN_SENSOR_PIN, INPUT);
  pinMode(IR_SENSOR_PIN, INPUT);

  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);
  liftStop();

  gasServo.attach(GAS_SERVO_PIN);
  rainServo.attach(RAIN_SERVO_PIN);

  gasServo.write(GAS_WINDOW_CLOSED);
  rainServo.write(RAIN_SERVO_HOME);

  Serial.println();
  Serial.println(F("===================================="));
  Serial.println(F(" ESP8266 SMART SAFETY AND LIFT SYSTEM"));
  Serial.println(F("===================================="));

  Serial.print(F("Gas threshold: "));
  Serial.println(GAS_THRESHOLD);

  Serial.print(F("Lift upward time: "));
  Serial.print(LIFT_UP_TIME);
  Serial.println(F(" ms"));

  Serial.print(F("Lift return time: "));
  Serial.print(LIFT_RETURN_TIME);
  Serial.println(F(" ms"));

  Serial.println(F("System ready."));
}

// ============== MAIN LOOP ==============
void loop() {
  // ----- GAS DETECTION -----
  int gasValue = analogRead(GAS_SENSOR_PIN);
  bool gasDetected = (gasValue >= GAS_THRESHOLD);

  digitalWrite(WARNING_LED_PIN, gasDetected ? HIGH : LOW);

  if (gasDetected && !gasWindowOpen) {
    gasServo.write(GAS_WINDOW_OPEN);
    gasWindowOpen = true;
    Serial.println(F("GAS DETECTED: Gas servo opening."));
  } else if (!gasDetected && gasWindowOpen) {
    gasServo.write(GAS_WINDOW_CLOSED);
    gasWindowOpen = false;
    Serial.println(F("Gas normal: Gas servo closing."));
  }

  // ----- RAIN DETECTION -----
  bool rainDetected =
      (digitalRead(RAIN_SENSOR_PIN) == RAIN_DETECTED_LEVEL);

  if (rainDetected && !rainServoActivated) {
    rainServo.write(RAIN_SERVO_ACTIVE);
    rainServoActivated = true;
    Serial.println(F("RAIN DETECTED: Rain servo activated."));
  } else if (!rainDetected && rainServoActivated) {
    rainServo.write(RAIN_SERVO_HOME);
    rainServoActivated = false;
    Serial.println(F("Rain cleared: Rain servo returned home."));
  }

  // ----- IR SENSOR AND LIFT -----
  bool objectDetected =
      (digitalRead(IR_SENSOR_PIN) == IR_DETECTED_LEVEL);

  updateLift(objectDetected);

  // ----- SERIAL MONITOR -----
  Serial.print(F("Gas: "));
  Serial.print(gasValue);

  Serial.print(F(" | Gas status: "));
  Serial.print(gasDetected ? F("DETECTED") : F("NORMAL"));

  Serial.print(F(" | Rain: "));
  Serial.print(rainDetected ? F("DETECTED") : F("NONE"));

  Serial.print(F(" | IR: "));
  Serial.print(objectDetected ? F("DETECTED") : F("NONE"));

  Serial.print(F(" | Gas servo: "));
  Serial.print(gasWindowOpen ? 90 : 0);

  Serial.print(F(" | Rain servo: "));
  Serial.print(rainServoActivated ? 180 : 0);

  Serial.print(F(" | Lift: "));

  switch (liftState) {
    case LIFT_MOVING_UP:
      Serial.println(F("MOVING UP"));
      break;

    case LIFT_AT_TOP:
      Serial.println(F("STOPPED AT TOP"));
      break;

    case LIFT_RETURNING:
      Serial.println(F("RETURNING DOWN"));
      break;

    case LIFT_LANDED:
      Serial.println(F("STOPPED AT LANDING"));
      break;
  }

  delay(50);
}
