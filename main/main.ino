
#include <Arduino.h>
#include <Servo.h>

// ============== GAS SENSOR ==============
constexpr uint8_t GAS_SENSOR_PIN = A0;
constexpr int GAS_THRESHOLD = 240;
constexpr uint8_t WARNING_LED_PIN = 12;  // D6 / GPIO12

// ============== RAIN SENSOR ==============
constexpr uint8_t RAIN_SENSOR_PIN = 5;   // D1 / GPIO5
constexpr uint8_t RAIN_DETECTED_LEVEL = LOW;

// ============== SERVOS ==============
constexpr uint8_t RAIN_SERVO_PIN = 4;    // D2 / GPIO4
constexpr uint8_t GAS_SERVO_PIN = 14;    // D5 / GPIO14

constexpr int SERVO_NORMAL_POSITION = 0;
constexpr int SERVO_ACTIVE_POSITION = 180;

Servo rainServo;
Servo gasServo;

// ============== IR SENSOR / LIFT ==============
constexpr uint8_t IR_SENSOR_PIN = 13;    // D7 / GPIO13
constexpr uint8_t IR_DETECTED_LEVEL = LOW;

// L298N motor driver
constexpr uint8_t MOTOR_IN1_PIN = 0;     // D3 / GPIO0
constexpr uint8_t MOTOR_IN2_PIN = 2;     // D4 / GPIO2

constexpr unsigned long LIFT_UP_TIME = 1800UL;    // 5 seconds
constexpr unsigned long LIFT_DOWN_TIME = 1800UL;  // 2 seconds
constexpr unsigned long LIFT_WAIT_TIME = 3000UL;  // 3 seconds

enum LiftState {
  LIFT_LANDED,
  MOVING_UP,
  WAITING_AT_TOP,
  WAITING_FOR_NEW_DETECTION,
  RETURNING,
  WAITING_AT_BOTTOM
};

LiftState liftState = LIFT_LANDED;
unsigned long liftStartTime = 0;
bool sensorCleared = false;

// ============== SOIL SENSOR / RELAY ==============
constexpr uint8_t SOIL_SENSOR_PIN = 3;   // GPIO3 / RX
constexpr uint8_t RELAY_PIN = 16;        // D0 / GPIO16

// Assumes HIGH means dry
constexpr uint8_t SOIL_DRY_LEVEL = HIGH;

// Active LOW relay
constexpr uint8_t RELAY_ON = LOW;
constexpr uint8_t RELAY_OFF = HIGH;


// ============== MOTOR CONTROL ==============

void stopLiftMotor() {
  digitalWrite(MOTOR_IN1_PIN, LOW);
  digitalWrite(MOTOR_IN2_PIN, LOW);
}

void moveLiftUp() {
  digitalWrite(MOTOR_IN1_PIN, HIGH);
  digitalWrite(MOTOR_IN2_PIN, LOW);
}

void moveLiftDown() {
  digitalWrite(MOTOR_IN1_PIN, LOW);
  digitalWrite(MOTOR_IN2_PIN, HIGH);
}


// ============== LIFT CONTROL ==============

void updateLift() {
  const unsigned long currentTime = millis();

  const bool irDetected =
      (digitalRead(IR_SENSOR_PIN) == IR_DETECTED_LEVEL);

  switch (liftState) {

    case LIFT_LANDED:
      stopLiftMotor();

      // First IR detection starts upward movement
      if (irDetected) {
        moveLiftUp();
        liftStartTime = currentTime;
        liftState = MOVING_UP;
      }
      break;

    case MOVING_UP:
      // Move upward for 5 seconds
      if (currentTime - liftStartTime >= LIFT_UP_TIME) {
        stopLiftMotor();
        liftStartTime = currentTime;
        sensorCleared = false;
        liftState = WAITING_AT_TOP;
      }
      break;

    case WAITING_AT_TOP:
      // Wait 3 seconds after moving up
      stopLiftMotor();

      if (currentTime - liftStartTime >= LIFT_WAIT_TIME) {
        liftState = WAITING_FOR_NEW_DETECTION;
      }
      break;

    case WAITING_FOR_NEW_DETECTION:
      stopLiftMotor();

      // The IR sensor must become inactive first
      if (!irDetected) {
        sensorCleared = true;
      }

      // A fresh detection starts downward movement
      if (sensorCleared && irDetected) {
        moveLiftDown();
        liftStartTime = currentTime;
        liftState = RETURNING;
      }
      break;

    case RETURNING:
      // Move downward for 2 seconds
      if (currentTime - liftStartTime >= LIFT_DOWN_TIME) {
        stopLiftMotor();
        liftStartTime = currentTime;
        liftState = WAITING_AT_BOTTOM;
      }
      break;

    case WAITING_AT_BOTTOM:
      // Wait 3 seconds after moving down
      stopLiftMotor();

      if (currentTime - liftStartTime >= LIFT_WAIT_TIME) {
        sensorCleared = false;
        liftState = LIFT_LANDED;
      }
      break;
  }
}


// ============== SETUP ==============

void setup() {
  pinMode(GAS_SENSOR_PIN, INPUT);

  pinMode(WARNING_LED_PIN, OUTPUT);
  digitalWrite(WARNING_LED_PIN, LOW);

  pinMode(RAIN_SENSOR_PIN, INPUT);
  pinMode(IR_SENSOR_PIN, INPUT);

  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);
  stopLiftMotor();

  pinMode(SOIL_SENSOR_PIN, INPUT);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);

  rainServo.attach(RAIN_SERVO_PIN);
  gasServo.attach(GAS_SERVO_PIN);

  rainServo.write(SERVO_NORMAL_POSITION);
  gasServo.write(SERVO_NORMAL_POSITION);
}


// ============== MAIN LOOP ==============

void loop() {

  // ---------- GAS SENSOR ----------
  const int gasValue = analogRead(GAS_SENSOR_PIN);

  if (gasValue >= GAS_THRESHOLD) {
    digitalWrite(WARNING_LED_PIN, HIGH);
    gasServo.write(SERVO_ACTIVE_POSITION);
  } else {
    digitalWrite(WARNING_LED_PIN, LOW);
    gasServo.write(SERVO_NORMAL_POSITION);
  }

  // ---------- RAIN SENSOR ----------
  const bool rainDetected =
      (digitalRead(RAIN_SENSOR_PIN) == RAIN_DETECTED_LEVEL);

  if (rainDetected) {
    rainServo.write(SERVO_ACTIVE_POSITION);
  } else {
    rainServo.write(SERVO_NORMAL_POSITION);
  }

  // ---------- SOIL SENSOR / RELAY ----------
  const bool soilIsDry =
      (digitalRead(SOIL_SENSOR_PIN) == SOIL_DRY_LEVEL);

  if (soilIsDry) {
    digitalWrite(RELAY_PIN, RELAY_ON);
  } else {
    digitalWrite(RELAY_PIN, RELAY_OFF);
  }

  // ---------- AUTOMATIC LIFT ----------
  updateLift();
}
