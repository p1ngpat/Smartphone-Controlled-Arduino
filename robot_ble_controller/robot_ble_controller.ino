#include <ArduinoBLE.h>

BLEService robotService("12345678-1234-5678-1234-56789abcdef0");

BLEByteCharacteristic movementCharacteristic(
  "12345678-1234-5678-1234-56789abcdef1",
  BLEWrite
);

void setup() {
  Serial.begin(9600);

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  BLE.setLocalName("RobotController");
  BLE.setAdvertisedService(robotService);

  robotService.addCharacteristic(movementCharacteristic);
  BLE.addService(robotService);

  BLE.advertise();

  Serial.println("BLE robot is advertising...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    while (central.connected()) {

      if (movementCharacteristic.written()) {
        char command = movementCharacteristic.value();

        Serial.print("Received: ");
        Serial.println(command);

        if (command == 'F') {
          Serial.println("Forward");
        }
        else if (command == 'B') {
          Serial.println("Backward");
        }
        else if (command == 'L') {
          Serial.println("Left");
        }
        else if (command == 'R') {
          Serial.println("Right");
        }
        else if (command == 'S') {
          Serial.println("Stop");
        }
      }
    }
  }
}