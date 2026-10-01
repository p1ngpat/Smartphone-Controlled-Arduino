import 'package:flutter/material.dart';

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

  void scanForRobot() {
    setState(() {
      connectionStatus = 'Scanning...';
    });

    // BLE scanning will be added later.
  }

  void sendCommand(String command) {
    setState(() {
      lastCommand = command;
    });

    // BLE write will be added later.
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
              onPressed: () => sendCommand('Forward'), // arduino F
              child: const Text('Forward'),
            ),

            const SizedBox(height: 16),

            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                ElevatedButton(
                  onPressed: () => sendCommand('Left'), // arduino L
                  child: const Text('Left'),
                ),

                const SizedBox(width: 16),

                ElevatedButton(
                  onPressed: () => sendCommand('Stop'), // arduino S
                  child: const Text('Stop'),
                ),

                const SizedBox(width: 16),

                ElevatedButton(
                  onPressed: () => sendCommand('Right'), // arduino R
                  child: const Text('Right'),
                ),
              ],
            ),

            const SizedBox(height: 16),

            ElevatedButton(
              onPressed: () => sendCommand('Backward'), // arduino B
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
