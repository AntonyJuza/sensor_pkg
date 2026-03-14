# sensor_pkg

`sensor_pkg` is a ROS 2 package that provides drivers, nodes, and launch scripts to interface with various hardware sensors. This package also includes a sensor simulator for testing and development without physical hardware.

## Supported Sensors

- **DHT11**: Digital Temperature and Humidity sensor
- **DHT22**: High accuracy Digital Temperature and Humidity sensor (includes Python implementation using `pigpio`)
- **Flame Sensor**: For fire/flame detection
- **MQ-2**: Gas/Smoke sensor

## Package Structure

- `src/`: C++ nodes and drivers for the sensors and the simulator.
- `scripts/`: Python nodes and scripts, notably `pigpio` based implementations for the DHT22.
- `launch/`: Launch files to start all hardware sensors or the simulator easily.
- `include/`: Header files for the C++ drivers.
- `config/`: Configuration files (if applicable).

## Dependencies

- ROS 2 (e.g., Humble/Iron/Jazzy)
- `rclcpp` and `rclpy`
- `std_msgs` and `sensor_msgs`
- `python3-pigpio` (required for python-based DHT22 functionality)

## Usage

### Using Real Sensors

To launch the real sensor nodes:

```bash
ros2 launch sensor_pkg sensors.launch.py
```

### Using the Simulator

If you don't have the hardware connected but want to test the rest of the software pipeline:

```bash
ros2 launch sensor_pkg simulator.launch.py
```

## License

This package is licensed under the Apache License 2.0.
