
#include <ArduinoBLE.h>

// BLE service and movement characteristic
BLEService robotService(
  "12345678-1234-5678-1234-56789abcdef0"
);

BLEByteCharacteristic movementCharacteristic(
  "12345678-1234-5678-1234-56789abcdef1",
  BLEWrite
);

// Ultrasonic sensor pins 
const int trigPin = 12;
const int echoPin = 11; 

// minimum stopping distance 
const float stopDistance = 20.0; 

// movement tracking
char currentMovement = 'S';

// Distance check interval (ms)
const unsigned long sensorInterval = 80; 
unsigned long lastSensorCheck = 0; 

// L298N Motor Driver Pins
const int ENA = 5;
const int IN1 = 7;
const int IN2 = 8;
const int IN3 = 9;
const int IN4 = 10;
const int ENB = 6;

// Motor speed (0-255)
const int motorSpeed = 180;

// Motor movement functions

void moveForward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  Serial.println("Forward");
}

void moveBackward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  Serial.println("Backward");
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  Serial.println("Left");
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  Serial.println("Right");
}


void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  Serial.println("Stop");
}

// Measure distance 
float getDistance(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  unsigned long duration =
      pulseIn(echoPin, HIGH, 25000UL);

  if (duration == 0) { 
    return -1;
  }
// convert echo duration to cm and divide 2 for round trip 
  return duration * .0343 / 2.0;

}

// Distance check 
bool forwardIsSafe(){
  float distance = getDistance();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

// No echo or obstacle too close 
  if (distance < 0 || distance <= stopDistance){
    return false;
  }
  return true;
}

void setup() {
  Serial.begin(9600);

  Serial.println("Starting setup...");

  // Configure motor pins
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Stop motors before starting BLE
  stopMotors();

  // Set motor speed
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);

  // Ultrasonic sensor setup 
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  digitalWrite(trigPin, LOW);


  Serial.println("Calling BLE.begin()...");

  if (!BLE.begin()) {
    Serial.println("BLE.begin() FAILED");
    stopMotors();
    while (1);
  }

  Serial.println("BLE.begin() succeeded");

  BLE.setLocalName("RobotController");
  BLE.setAdvertisedService(robotService);

  robotService.addCharacteristic(
    movementCharacteristic
  );

  BLE.addService(robotService);

  Serial.println("Starting advertising...");

  BLE.advertise();

  Serial.println("BLE robot is advertising...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to: ");
    Serial.println(central.address());

    // Motors remain stopped until a command arrives
    stopMotors();
    currentMovement = 'S';
    lastSensorCheck = millis();


    while (central.connected()) {

      if (movementCharacteristic.written()) {
        // Process movement commands
        char command = movementCharacteristic.value();

        Serial.print("Received: ");
        Serial.println(command);

        // check if distance before moving 
        if (command == 'F') {
          if (forwardIsSafe()){
            moveForward();
            currentMovement = 'F';
            lastSensorCheck = millis();
          }
          else{
            stopMotors();
            currentMovement = 'S';
            Serial.println("Forward blocked");
          }
        }
        else if (command == 'B') {
          moveBackward();
        }
        else if (command == 'L') {
          turnLeft();
        }
        else if (command == 'R') {
          turnRight();
        }
        else if (command == 'S') {
          stopMotors();
        }
        else {
          stopMotors();
          Serial.println("Unknown command");
        }
      }
      // monitor forward movement f
      if (currentMovement == 'F' && millis() - lastSensorCheck >= sensorInterval){
        
        lastSensorCheck = millis();

        if (!forwardIsSafe()){
          stopMotors();
          currentMovement = 'S';

          Serial.println(
            "Obstacle - Auto stop"
          );
        }

      }
    }

    // Safety: stop motors on BLE disconnect
    stopMotors();

    Serial.println("BLE disconnected");
  }
}
