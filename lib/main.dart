import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';

void main() {
  runApp(const RobotControllerApp());
}

class RobotControllerApp extends StatelessWidget {
  const RobotControllerApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'Robot Controller',
      theme: ThemeData(useMaterial3: true),
      home: const RobotControllerPage(),
    );
  }
}

class RobotControllerPage extends StatefulWidget {
  const RobotControllerPage({super.key});

  @override
  State<RobotControllerPage> createState() => _RobotControllerPageState();
}

class _RobotControllerPageState extends State<RobotControllerPage> {
  String connectionStatus = 'Disconnected';
  String lastCommand = 'None';

  BluetoothDevice? connectedDevice;
  BluetoothCharacteristic? movementCharacteristic;

  //ask Android for BLE permissions
  Future<bool> requestBluetoothPermissions() async {
    Map<Permission, PermissionStatus> statuses = await [
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.locationWhenInUse,
    ].request();

    bool scanGranted = statuses[Permission.bluetoothScan]?.isGranted ?? false;

    bool connectGranted =
        statuses[Permission.bluetoothConnect]?.isGranted ?? false;

    return scanGranted && connectGranted;
  }

  //scan for BLE device and connect to it
  Future<void> scanForRobot() async {
    bool permissionGranted = await requestBluetoothPermissions();

    if (!permissionGranted) {
      setState(() {
        connectionStatus = "Bluetooth permission denied";
      });
      return;
    }

    setState(() {
      connectionStatus = 'Scanning...';
    });

    try {
      await FlutterBluePlus.startScan(timeout: const Duration(seconds: 10));

      FlutterBluePlus.scanResults.listen((results) async {
        for (ScanResult result in results) {
          if (result.device.platformName == 'RobotController') {
            await FlutterBluePlus.stopScan();

            setState(() {
              connectionStatus = 'Connecting...';
            });

            connectedDevice = result.device;

            // Connect to Arduino
            await connectedDevice!.connect();

            // Discover services after connection
            List<BluetoothService> services = await connectedDevice!
                .discoverServices();

            // Find robot service and movement characteristic
            for (BluetoothService service in services) {
              if (service.uuid.toString().toLowerCase() ==
                  '12345678-1234-5678-1234-56789abcdef0') {
                for (BluetoothCharacteristic characteristic
                    in service.characteristics) {
                  if (characteristic.uuid.toString().toLowerCase() ==
                      '12345678-1234-5678-1234-56789abcdef1') {
                    movementCharacteristic = characteristic;

                    debugPrint('Movement characteristic found');
                  }
                }
              }
            }

            // Update connection status
            if (movementCharacteristic != null) {
              setState(() {
                connectionStatus = 'Connected';
              });
            } else {
              setState(() {
                connectionStatus = 'Characteristic not found';
              });
            }

            break;
          }
        }
      });
    } catch (e) {
      setState(() {
        connectionStatus = 'Connection failed';
      });

      debugPrint('BLE Error: $e');
    }
  }

  Future<void> sendCommand(String command) async {
    // Make sure the BLE movement characteristic has been found
    if (movementCharacteristic == null) {
      debugPrint('Movement characteristic not available');
      return;
    }

    try {
      // Convert the command character into its byte value
      List<int> value = command.codeUnits;

      // Send the command to the Arduino
      await movementCharacteristic!.write(value, withoutResponse: false);

      // Update the UI after a successful write
      setState(() {
        lastCommand = command;
      });

      debugPrint('Sent command: $command');
    } catch (e) {
      debugPrint('BLE write error: $e');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Smartphone Robot Controller'),
        centerTitle: true,
      ),
      body: Padding(
        padding: const EdgeInsets.all(24.0),
        child: Column(
          children: [
            const SizedBox(height: 20),

            ElevatedButton(
              onPressed: scanForRobot,
              child: const Text('Scan for Robot'),
            ),

            const SizedBox(height: 16),

            Text(
              'Status: $connectionStatus',
              style: const TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),

            const SizedBox(height: 50),

            ElevatedButton(
              onPressed: () => sendCommand('F'), // arduino F
              child: const Text('Forward'),
            ),

            const SizedBox(height: 16),

            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                ElevatedButton(
                  onPressed: () => sendCommand('L'), // arduino L
                  child: const Text('Left'),
                ),

                const SizedBox(width: 16),

                ElevatedButton(
                  onPressed: () => sendCommand('S'), // arduino S
                  child: const Text('Stop'),
                ),

                const SizedBox(width: 16),

                ElevatedButton(
                  onPressed: () => sendCommand('R'), // arduino R
                  child: const Text('Right'),
                ),
              ],
            ),

            const SizedBox(height: 16),

            ElevatedButton(
              onPressed: () => sendCommand('B'), // arduino B
              child: const Text('Backward'),
            ),

            const SizedBox(height: 40),
            // test case for commands
            Text(
              'Last Command: $lastCommand',
              style: const TextStyle(fontSize: 18),
            ),
          ],
        ),
      ),
    );
  }
}
