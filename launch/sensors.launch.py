from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # DHT11 Temperature & Humidity Sensor
        Node(
            package='sensor_pkg',
            executable='dht11_node',
            name='dht11_node',
            output='screen',
            parameters=[{
                'pin': 7,  # WiringPi pin 7 (GPIO 4)
                'publish_rate': 1.0  # 1 Hz
            }]
        ),
        
        # MQ2 Gas Sensor
        Node(
            package='sensor_pkg',
            executable='mq2_node',
            name='mq2_node',
            output='screen',
            parameters=[{
                'analog_channel': 0,  # MCP3008 channel 0
                'digital_pin': 0,     # WiringPi pin 0 (GPIO 17)
                'publish_rate': 2.0,  # 2 Hz
                'gas_threshold': 300.0  # PPM
            }]
        ),
        
        # Flame Sensor
        Node(
            package='sensor_pkg',
            executable='flame_node',
            name='flame_node',
            output='screen',
            parameters=[{
                'digital_pin': 1,  # WiringPi pin 1 (GPIO 18)
                'analog_channel': 1,  # MCP3008 channel 1 (optional)
                'publish_rate': 5.0  # 5 Hz
            }]
        ),
    ])