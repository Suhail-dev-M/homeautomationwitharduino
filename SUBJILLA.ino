#include <Servo.h>

// SUBJILLA Smart Home Automation
// Arduino UNO — roof button and roof servo removed.

// ========== INPUTS ==========
const int LDR_PIN = A0;
const int GAS_PIN = A1;
const int WATER_LEVEL_PIN = A2;
const int LIFT_SENSOR_PIN = 12;

// ========== OUTPUTS ==========
const int PUMP_IN1 = 4;
const int STREET_LED = 5;
const int FAN_IN1 = 6;
const int PUMP_IN2 = 7;
const int FAN_IN2 = 8;

const int WINDOW_PIN = 9;
const int LIFT_PIN = 11;

const int GAS_LED = A3;
const int TANK_FULL_LED = A4;

// ========== CALIBRATE THESE VALUES ==========
// Example values only. Read your sensor in Serial Monitor
// before connecting motor power.
const int WATER_LOW_THRESHOLD = 300;
const int WATER_FULL_THRESHOLD = 600;

const int BRIGHT_THRESHOLD = 930;
const int DARK_THRESHOLD = 1000;

const int GAS_ON_THRESHOLD = 450;
const int GAS_OFF_THRESHOLD = 400;

bool pumpRunning = false;
bool gasDetected = false;

// ========== SERVOS ==========
Servo windowServo;
Servo liftServo;

const int WINDOW_CLOSED = 0;
const int WINDOW_OPEN = 90;
const int LIFT_DOWN = 0;
const int LIFT_UP = 180;

int windowAngle = WINDOW_CLOSED;
int windowTarget = WINDOW_CLOSED;
int liftAngle = LIFT_DOWN;
int liftTarget = LIFT_DOWN;

const unsigned long SERVO_INTERVAL = 35;
const unsigned long CONFIRM_TIME = 300;

unsigned long windowTimer = 0;
unsigned long liftTimer = 0;
unsigned long liftChangeTime = 0;

int liftCandidate = LOW;
int liftConfirmed = LOW;

// ========== MOTOR CONTROL ==========
void pumpOn() {
  digitalWrite(PUMP_IN1, HIGH);
  digitalWrite(PUMP_IN2, LOW);
}

void pumpOff() {
  digitalWrite(PUMP_IN1, LOW);
  digitalWrite(PUMP_IN2, LOW);
}

void fanOn() {
  digitalWrite(FAN_IN1, HIGH);
  digitalWrite(FAN_IN2, LOW);
}

void fanOff() {
  digitalWrite(FAN_IN1, LOW);
  digitalWrite(FAN_IN2, LOW);
}

// ========== SMOOTH SERVO MOVEMENT ==========
void moveServoSmoothly(
  Servo &motor,
  int &currentAngle,
  int targetAngle,
  unsigned long &timer
) {
  unsigned long now = millis();

  if (now - timer < SERVO_INTERVAL) return;
  if (currentAngle == targetAngle) return;

  timer = now;

  if (currentAngle < targetAngle) {
    currentAngle++;
  } else {
    currentAngle--;
  }

  motor.write(currentAngle);
}

// ========== SETUP ==========
void setup() {
  Serial.begin(9600);

  pinMode(STREET_LED, OUTPUT);
  pinMode(GAS_LED, OUTPUT);
  pinMode(TANK_FULL_LED, OUTPUT);

  pinMode(PUMP_IN1, OUTPUT);
  pinMode(PUMP_IN2, OUTPUT);
  pinMode(FAN_IN1, OUTPUT);
  pinMode(FAN_IN2, OUTPUT);

  pinMode(LIFT_SENSOR_PIN, INPUT);

  // Keep motor outputs off at startup.
  pumpOff();
  fanOff();

  digitalWrite(GAS_LED, LOW);
  digitalWrite(TANK_FULL_LED, LOW);
  analogWrite(STREET_LED, 0);

  windowServo.attach(WINDOW_PIN);
  liftServo.attach(LIFT_PIN);

  windowServo.write(WINDOW_CLOSED);
  liftServo.write(LIFT_DOWN);

  liftCandidate = digitalRead(LIFT_SENSOR_PIN);
  liftConfirmed = liftCandidate;
  liftChangeTime = millis();

  Serial.println(F("SUBJILLA Smart Home Started"));
  Serial.println(F("Water sensor signal: A2"));
  Serial.println(F("Calibrate water thresholds before using the pump."));
}

// ========== MAIN LOOP ==========
void loop() {
  unsigned long now = millis();

  // 1. AUTOMATIC STREET LIGHT
  int lightLevel = analogRead(LDR_PIN);
  int brightness = 0;

  if (lightLevel <= BRIGHT_THRESHOLD) {
    brightness = 0;
  } else if (lightLevel >= DARK_THRESHOLD) {
    brightness = 255;
  } else {
    brightness = map(
      lightLevel,
      BRIGHT_THRESHOLD,
      DARK_THRESHOLD,
      0,
      255
    );
  }

  analogWrite(STREET_LED, constrain(brightness, 0, 255));

  // 2. WATER TANK AUTOMATION
  // This assumes a higher analog reading means more water.
  // Verify this with your sensor; reverse the comparisons if needed.
  int waterLevel = analogRead(WATER_LEVEL_PIN);

  if (waterLevel <= WATER_LOW_THRESHOLD) {
    pumpRunning = true;
  }

  if (waterLevel >= WATER_FULL_THRESHOLD) {
    pumpRunning = false;
  }

  if (pumpRunning) {
    pumpOn();
  } else {
    pumpOff();
  }

  bool tankFull = waterLevel >= WATER_FULL_THRESHOLD;
  digitalWrite(TANK_FULL_LED, tankFull ? HIGH : LOW);

  // 3. GAS DETECTION (demonstration only, not a safety system)
  int gasLevel = analogRead(GAS_PIN);

  if (!gasDetected && gasLevel >= GAS_ON_THRESHOLD) {
    gasDetected = true;
    Serial.println(F("WARNING: GAS THRESHOLD REACHED"));
  }

  if (gasDetected && gasLevel <= GAS_OFF_THRESHOLD) {
    gasDetected = false;
    Serial.println(F("Gas reading returned below reset threshold"));
  }

  digitalWrite(GAS_LED, gasDetected ? HIGH : LOW);

  if (gasDetected) {
    fanOn();
    windowTarget = WINDOW_OPEN;
  } else {
    fanOff();
    windowTarget = WINDOW_CLOSED;
  }

  // 4. LIFT SENSOR
  int liftReading = digitalRead(LIFT_SENSOR_PIN);

  if (liftReading != liftCandidate) {
    liftCandidate = liftReading;
    liftChangeTime = now;
  }

  if (now - liftChangeTime >= CONFIRM_TIME) {
    liftConfirmed = liftCandidate;
  }

  liftTarget = (liftConfirmed == HIGH) ? LIFT_UP : LIFT_DOWN;

  // 5. GRADUAL SERVO MOVEMENT
  moveServoSmoothly(
    windowServo, windowAngle, windowTarget, windowTimer
  );

  moveServoSmoothly(
    liftServo, liftAngle, liftTarget, liftTimer
  );

  // 6. SERIAL MONITOR STATUS
  static unsigned long lastPrint = 0;

  if (now - lastPrint >= 500) {
    lastPrint = now;

    Serial.print(F("Light: "));
    Serial.print(lightLevel);

    Serial.print(F(" | Brightness: "));
    Serial.print(brightness);

    Serial.print(F(" | Water: "));
    Serial.print(waterLevel);

    Serial.print(F(" | Pump: "));
    Serial.print(pumpRunning ? F("ON") : F("OFF"));

    Serial.print(F(" | Tank full: "));
    Serial.print(tankFull ? F("YES") : F("NO"));

    Serial.print(F(" | Gas: "));
    Serial.print(gasLevel);

    Serial.print(F(" | Gas detected: "));
    Serial.print(gasDetected ? F("YES") : F("NO"));

    Serial.print(F(" | Window angle: "));
    Serial.print(windowAngle);

    Serial.print(F(" | Lift angle: "));
    Serial.println(liftAngle);
  }
}
