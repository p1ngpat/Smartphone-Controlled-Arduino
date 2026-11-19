#include <ArduinoBLE.h>

// BLE service
BLEService robotService(
  "12345678-1234-5678-1234-56789abcdef0"
);

// Receives movement commands from Flutter
BLEByteCharacteristic movementCharacteristic(
  "12345678-1234-5678-1234-56789abcdef1",
  BLEWrite
);

// Sends ultrasonic readings to Flutter
BLEStringCharacteristic sensorCharacteristic(
  "12345678-1234-5678-1234-56789abcdef2",
  BLERead | BLENotify,
  32
);

// Ultrasonic sensor pins
const int trigPin = 12;
const int echoPin = 11;

// Minimum stopping distance in cm
const float stopDistance = 20.0;

// Movement tracking
char currentMovement = 'S';

// Distance check interval (ms)
const unsigned long sensorInterval = 80;
unsigned long lastSensorCheck = 0;

// Rolling median filter settings
const int sampleSize = 3;
float distanceSamples[sampleSize];

int sampleIndex = 0;
int sampleCount = 0;

// L298N Motor Driver Pins
const int ENA = 5;
const int IN1 = 7;
const int IN2 = 8;
const int IN3 = 9;
const int IN4 = 10;
const int ENB = 6;

// Motor speed (0-255)
const int motorSpeed = 180;


// --------------------------
// Motor movement functions
// --------------------------

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


// --------------------------
// Ultrasonic sensor
// --------------------------

// Measure distance using HC-SR04
float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Wait up to 25 ms for echo
  unsigned long duration =
      pulseIn(echoPin, HIGH, 25000UL);

  // No echo received
  if (duration == 0) {
    return -1;
  }

  // Convert round-trip travel time to cm
  return duration * 0.0343 / 2.0;
}


// Calculate median of last 3 readings
float getMedianDistance() {
  // Require 3 readings before using filter
  if (sampleCount < sampleSize) {
    return -1;
  }

  float a = distanceSamples[0];
  float b = distanceSamples[1];
  float c = distanceSamples[2];

  // Sort readings into ascending order
  if (a > b) {
    float temp = a;
    a = b;
    b = temp;
  }

  if (b > c) {
    float temp = b;
    b = c;
    c = temp;
  }

  if (a > b) {
    float temp = a;
    a = b;
    b = temp;
  }

  // Return middle value
  return b;
}


// Collect a reading and send it to Flutter
void updateDistanceSample() {
  float rawDistance = getDistance();

  // Store latest reading in circular buffer
  distanceSamples[sampleIndex] = rawDistance;

  sampleIndex = (sampleIndex + 1) % sampleSize;

  if (sampleCount < sampleSize) {
    sampleCount++;
  }

  // Calculate filtered distance
  float medianDistance = getMedianDistance();

  // Format raw and filtered readings
  String sensorData =
      String(rawDistance, 1) + "," +
      String(medianDistance, 1);

  // Transmit through BLE notifications
  sensorCharacteristic.writeValue(sensorData);

  // Print readings for USB debugging
  Serial.print("Raw: ");
  Serial.print(rawDistance);
  Serial.print(" cm | Median: ");
  Serial.print(medianDistance);
  Serial.println(" cm");
}


// Check whether forward movement is safe
bool forwardIsSafe() {
  float distance = getMedianDistance();

  // Invalid or incomplete readings are unsafe
  if (distance < 0) {
    Serial.println("Sensor unavailable");
    return false;
  }

  // Obstacle detected
  if (distance <= stopDistance) {
    Serial.println("Obstacle detected");
    return false;
  }

  return true;
}


// --------------------------
// Arduino setup
// --------------------------

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

  // Stop motors before BLE starts
  stopMotors();

  // Set motor speed
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);

  // Configure ultrasonic sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  digitalWrite(trigPin, LOW);

  // Start BLE
  Serial.println("Calling BLE.begin()...");

  if (!BLE.begin()) {
    Serial.println("BLE.begin() FAILED");
    stopMotors();
    while (1);
  }

  Serial.println("BLE.begin() succeeded");

  BLE.setLocalName("RobotController");
  BLE.setAdvertisedService(robotService);

  // Register both BLE characteristics
  robotService.addCharacteristic(
    movementCharacteristic
  );

  robotService.addCharacteristic(
    sensorCharacteristic
  );

  BLE.addService(robotService);

  // Initialize telemetry characteristic
  sensorCharacteristic.writeValue("-1,-1");

  // Advertise robot BLE service
  BLE.advertise();

  Serial.println("BLE robot is advertising...");
}


// --------------------------
// Main loop
// --------------------------

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to: ");
    Serial.println(central.address());

    // Stop until movement command arrives
    stopMotors();
    currentMovement = 'S';

    // Reset median filter
    sampleIndex = 0;
    sampleCount = 0;

    lastSensorCheck = millis();

    while (central.connected()) {

      // Read incoming movement command
      if (movementCharacteristic.written()) {
        char command =
            movementCharacteristic.value();

        Serial.print("Received: ");
        Serial.println(command);

        if (command == 'F') {

          if (forwardIsSafe()) {
            moveForward();
            currentMovement = 'F';
          }
          else {
            stopMotors();
            currentMovement = 'S';
            Serial.println("Forward blocked");
          }

        }
        else if (command == 'B') {
          moveBackward();
          currentMovement = 'B';
        }
        else if (command == 'L') {
          turnLeft();
          currentMovement = 'L';
        }
        else if (command == 'R') {
          turnRight();
          currentMovement = 'R';
        }
        else if (command == 'S') {
          stopMotors();
          currentMovement = 'S';
        }
        else {
          stopMotors();
          currentMovement = 'S';
          Serial.println("Unknown command");
        }
      }

      // Update sensor at regular intervals
      if (millis() - lastSensorCheck >= sensorInterval) {

        lastSensorCheck = millis();

        // Read sensor, filter, and notify Flutter
        updateDistanceSample();

        // Monitor safety during forward movement
        if (currentMovement == 'F') {

          if (!forwardIsSafe()) {
            stopMotors();
            currentMovement = 'S';

            Serial.println("Obstacle - Auto stop");
          }
        }
      }
    }

    // BLE disconnect safety
    stopMotors();
    currentMovement = 'S';

    Serial.println("BLE disconnected");
  }
}
