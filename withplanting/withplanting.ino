#include <Arduino.h>
#include <Servo.h>

// ============== GAS SENSOR ==============
constexpr uint8_t GAS_SENSOR_PIN = A0;
constexpr int GAS_THRESHOLD = 240;
constexpr uint8_t WARNING_LED_PIN = 12;  // GPIO12 (D6)

// ============== RAIN SENSOR ==============
constexpr uint8_t RAIN_SENSOR_PIN = 5;   // GPIO5 (D1)
constexpr uint8_t RAIN_DETECTED_LEVEL = LOW;

// ============== SERVOS ==============
constexpr uint8_t GAS_SERVO_PIN = 14;    // GPIO14 (D5)
constexpr uint8_t RAIN_SERVO_PIN = 4;    // GPIO4 (D2)

constexpr int GAS_WINDOW_CLOSED = 0;
constexpr int GAS_WINDOW_OPEN = 180;
constexpr int RAIN_SERVO_HOME = 0;
constexpr int RAIN_SERVO_ACTIVE = 180;

Servo gasServo;
Servo rainServo;

bool gasWindowOpen = false;
bool rainServoActivated = false;

// ============== IR SENSOR ==============
constexpr uint8_t IR_SENSOR_PIN = 13;    // GPIO13 (D7)
constexpr uint8_t IR_DETECTED_LEVEL = LOW;

// ============== PLANT WATERING ==============
// Soil sensor digital output uses GPIO3 (RX).
// Avoid Serial input while using GPIO3 as a sensor input.
constexpr uint8_t SOIL_SENSOR_PIN = 3;   // GPIO3 (RX)
constexpr uint8_t WATER_RELAY_PIN = 16; // GPIO16 (D0)

// Adjust these after testing your sensor and relay module.
constexpr uint8_t SOIL_DRY_LEVEL = HIGH;
constexpr uint8_t RELAY_ON = LOW;   // Common active-LOW relay module
constexpr uint8_t RELAY_OFF = HIGH;

bool pumpIsOn = false;

// ============== L298N MOTOR ==============
// Channel B: connect motor to OUT3 and OUT4.
// Keep ENB enabled (jumper fitted, if your board has one).
constexpr uint8_t MOTOR_IN3 = 0;  // GPIO0 (D3)
constexpr uint8_t MOTOR_IN4 = 2;  // GPIO2 (D4)

// Independent lift timings
constexpr unsigned long LIFT_UP_TIME = 5000UL;
constexpr unsigned long LIFT_RETURN_TIME = 2000UL;

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
    if (liftState == LIFT_LANDED || liftState == LIFT_RETURNING) {
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
    if (liftState == LIFT_MOVING_UP || liftState == LIFT_AT_TOP) {
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

// ============== PLANT WATERING LOGIC ==============
void updatePlantWatering() {
  bool soilIsDry = (digitalRead(SOIL_SENSOR_PIN) == SOIL_DRY_LEVEL);

  if (soilIsDry && !pumpIsOn) {
    digitalWrite(WATER_RELAY_PIN, RELAY_ON);
    pumpIsOn = true;
    Serial.println(F("SOIL DRY: Water pump ON"));
  } else if (!soilIsDry && pumpIsOn) {
    digitalWrite(WATER_RELAY_PIN, RELAY_OFF);
    pumpIsOn = false;
    Serial.println(F("SOIL MOIST: Water pump OFF"));
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

  // GPIO3 is RX; don't send serial data to the ESP8266 while using this pin.
  pinMode(SOIL_SENSOR_PIN, INPUT);

  pinMode(WATER_RELAY_PIN, OUTPUT);
  digitalWrite(WATER_RELAY_PIN, RELAY_OFF);

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
  Serial.println(F("Plant watering enabled."));
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

  // ----- PLANT WATERING -----
  updatePlantWatering();

  // ----- SERIAL MONITOR -----
  Serial.print(F("Gas: "));
  Serial.print(gasValue);
  Serial.print(F(" | Gas status: "));
  Serial.print(gasDetected ? F("DETECTED") : F("NORMAL"));
  Serial.print(F(" | Rain: "));
  Serial.print(rainDetected ? F("DETECTED") : F("NONE"));
  Serial.print(F(" | IR: "));
  Serial.print(objectDetected ? F("DETECTED") : F("NONE"));
  Serial.print(F(" | Soil: "));
  Serial.print(digitalRead(SOIL_SENSOR_PIN) == SOIL_DRY_LEVEL ? F("DRY") : F("MOIST"));
  Serial.print(F(" | Pump: "));
  Serial.print(pumpIsOn ? F("ON") : F("OFF"));
  Serial.print(F(" | Gas servo: "));
  Serial.print(gasWindowOpen ? 180 : 0);
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

  delay(200);
}
