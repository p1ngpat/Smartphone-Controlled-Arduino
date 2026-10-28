const int trigPin = 12; 
const int echoPin = 11; 


void setup() {
  Serial.begin(9600);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  digitalWrite(trigPin, LOW);

  Serial.println("UltraSonic Sensor Test");

}

void loop() {
  //send pulse 
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  // Measure reflected pulse duration
  unsigned long duration =
      pulseIn(echoPin, HIGH, 30000UL);

  if (duration == 0) {
    Serial.println("No echo detected");
  } else {
    // Convert round-trip time to centimeters
    float distance = duration * 0.0343 / 2.0;

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  }

  delay(100);
}

