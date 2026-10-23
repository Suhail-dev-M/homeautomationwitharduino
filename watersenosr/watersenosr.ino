
const int WATER_SENSOR_PIN = A1;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int waterLevel = analogRead(WATER_SENSOR_PIN);

  Serial.print("Water sensor value: ");
  Serial.println(waterLevel);

  delay(500);
}
