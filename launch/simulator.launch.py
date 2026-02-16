from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    """
    Launch file for sensor simulator (no hardware required)
    Useful for testing and development without Raspberry Pi hardware
    """
    return LaunchDescription([
        Node(
            package='sensor_pkg',
            executable='sensor_simulator',
            name='sensor_simulator',
            output='screen'
        ),
    ])