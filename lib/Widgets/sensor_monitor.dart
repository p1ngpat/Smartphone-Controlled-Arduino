import 'package:flutter/material.dart';

class SensorMonitor extends StatelessWidget {
  final double rawDistance;
  final double filteredDistance;

  const SensorMonitor({
    super.key,
    required this.rawDistance,
    required this.filteredDistance,
  });

  @override
  Widget build(BuildContext context) {
    bool obstacleDetected =
        filteredDistance >= 0 && filteredDistance <= 20.0;

    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          children: [
            const Text(
              'Ultrasonic Sensor',
              style: TextStyle(
                fontSize: 20,
                fontWeight: FontWeight.bold,
              ),
            ),

            const SizedBox(height: 12),

            Text(
              rawDistance < 0
                  ? 'Raw Distance: --'
                  : 'Raw Distance: ${rawDistance.toStringAsFixed(1)} cm',
              style: const TextStyle(fontSize: 16),
            ),

            const SizedBox(height: 8),

            Text(
              filteredDistance < 0
                  ? 'Filtered Distance: --'
                  : 'Filtered Distance: ${filteredDistance.toStringAsFixed(1)} cm',
              style: const TextStyle(fontSize: 16),
            ),

            const SizedBox(height: 12),

            Text(
              filteredDistance < 0
                  ? 'Sensor Unavailable'
                  : obstacleDetected
                  ? 'Obstacle Detected'
                  : 'Clear Path',
              style: TextStyle(
                fontSize: 18,
                fontWeight: FontWeight.bold,
                color: filteredDistance < 0 || obstacleDetected
                    ? Colors.red
                    : Colors.green,
              ),
            ),
          ],
        ),
      ),
    );
  }
}