
const int LDR_PIN = A0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int value = analogRead(LDR_PIN);
  Serial.println(value);
  delay(500);
}
