
 // ========== STREETLIGHT ==========
const int LDR_PIN = A0;
const int STREET_LIGHT_PIN = 9;

const int DARK_LEVEL = 1000;
const int LIGHT_LEVEL = 950;

// ========== WATER TANK ==========
const int WATER_SENSOR_PIN = A1;

const int RED_LED = 4;
const int YELLOW_LED = 5;
const int GREEN_LED = 6;

const int MOTOR_IN1 = 7;
const int MOTOR_IN2 = 8;

// Temporary thresholds: calibrate using actual tank readings.
const int WATER_LOW_LEVEL = 500;
const int WATER_FULL_LEVEL = 595;

bool pumpRunning = false;

// ========== PUMP CONTROL ==========
void pumpOn() {
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  pumpRunning = true;
}

void pumpOff() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  pumpRunning = false;
}

// ========== SETUP ==========
void setup() {
  Serial.begin(9600);

  pinMode(STREET_LIGHT_PIN, OUTPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);

  pumpOff();
}

// ========== MAIN LOOP ==========
void loop() {

  // ----- 1. AUTOMATIC STREETLIGHT -----
  int lightLevel = analogRead(LDR_PIN);
  int brightness = 0;

  if (lightLevel >= DARK_LEVEL) {
    brightness = 255;
  } else if (lightLevel > LIGHT_LEVEL) {
    brightness = map(
      lightLevel,
      LIGHT_LEVEL,
      DARK_LEVEL,
      0,
      255
    );
  }

  brightness = constrain(brightness, 0, 255);
  analogWrite(STREET_LIGHT_PIN, brightness);

  // ----- 2. AUTOMATIC WATER TANK -----
  int waterLevel = analogRead(WATER_SENSOR_PIN);

  if (waterLevel >= WATER_FULL_LEVEL) {
    pumpOff();
  } else {
    pumpOn();
  }

  if (waterLevel < WATER_LOW_LEVEL) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
  } else if (waterLevel < WATER_FULL_LEVEL) {
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
  }

  // ----- DEBUG INFORMATION -----
  Serial.print("LDR: ");
  Serial.print(lightLevel);

  Serial.print(" | Brightness: ");
  Serial.print(brightness);

  Serial.print(" | Water: ");
  Serial.print(waterLevel);

  Serial.print(" | Pump: ");
  Serial.println(pumpRunning ? "ON" : "OFF");

  delay(300);
}
