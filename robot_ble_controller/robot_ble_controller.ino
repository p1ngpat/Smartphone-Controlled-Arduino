#include <ArduinoBLE.h>


BLEService robotService("12345678-1234-5678-1234-56789abcdef0");

BLEByteCharacteristic movementCharacteristic(
        "12345678-1234-5678-1234-56789abcdef1",
        BLEWrite
);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Starting setup...");

  Serial.println("Calling BLE.begin()...");

  if (!BLE.begin()) {
    Serial.println("BLE.begin() FAILED");
    while (1);
  }

  Serial.println("BLE.begin() succeeded");

  BLE.setLocalName("RobotController");
  BLE.setAdvertisedService(robotService);

  robotService.addCharacteristic(movementCharacteristic);
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

    Serial.println("BLE disconnected");
  }
}