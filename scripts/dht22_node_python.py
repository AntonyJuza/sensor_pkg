#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Temperature, RelativeHumidity
import pigpio
import time
import atexit

# -----------------------------
# DHT22 driver class (embedded)
# -----------------------------
class sensor:
    """
    DHT22 driver class (from original DHT22.py)
    """
    def __init__(self, pi, gpio, LED=None, power=None):
        self.pi = pi
        self.gpio = gpio
        self.LED = LED
        self.power = power

        if power is not None:
            pi.write(power, 1)  # Power on
            time.sleep(2)

        self.powered = True
        self.cb = None
        atexit.register(self.cancel)

        self.bad_CS = 0
        self.bad_SM = 0
        self.bad_MM = 0
        self.bad_SR = 0
        self.no_response = 0
        self.MAX_NO_RESPONSE = 2

        self.rhum = -999
        self.temp = -999
        self.tov = None

        self.high_tick = 0
        self.bit = 40
        self.hH = self.hL = self.tH = self.tL = self.CS = 0

        pi.set_pull_up_down(gpio, pigpio.PUD_OFF)
        pi.set_watchdog(gpio, 0)
        self.cb = pi.callback(gpio, pigpio.EITHER_EDGE, self._cb)

    def _cb(self, gpio, level, tick):
        diff = pigpio.tickDiff(self.high_tick, tick)

        if level == 0:
            val = 1 if diff >= 50 else 0
            if diff >= 200:
                self.CS = 256  # Force bad checksum

            if self.bit >= 40:
                self.bit = 40
            elif self.bit >= 32:  # checksum byte
                self.CS = (self.CS << 1) + val
                if self.bit == 39:
                    self.pi.set_watchdog(self.gpio, 0)
                    self.no_response = 0
                    total = self.hH + self.hL + self.tH + self.tL
                    if (total & 255) == self.CS:
                        self.rhum = ((self.hH << 8) + self.hL) * 0.1
                        mult = -0.1 if self.tH & 128 else 0.1
                        if self.tH & 128:
                            self.tH &= 127
                        self.temp = ((self.tH << 8) + self.tL) * mult
                        self.tov = time.time()
                        if self.LED is not None:
                            self.pi.write(self.LED, 0)
                    else:
                        self.bad_CS += 1
            elif self.bit >= 24:  # temp low byte
                self.tL = (self.tL << 1) + val
            elif self.bit >= 16:  # temp high byte
                self.tH = (self.tH << 1) + val
            elif self.bit >= 8:  # humidity low byte
                self.hL = (self.hL << 1) + val
            elif self.bit >= 0:  # humidity high byte
                self.hH = (self.hH << 1) + val
            self.bit += 1

        elif level == 1:
            self.high_tick = tick
            if diff > 250000:
                self.bit = -2
                self.hH = self.hL = self.tH = self.tL = self.CS = 0
        else:  # timeout
            self.pi.set_watchdog(self.gpio, 0)
            if self.bit < 8:
                self.bad_MM += 1
                self.no_response += 1
                if self.no_response > self.MAX_NO_RESPONSE:
                    self.no_response = 0
                    self.bad_SR += 1
                    if self.power is not None:
                        self.powered = False
                        self.pi.write(self.power, 0)
                        time.sleep(2)
                        self.pi.write(self.power, 1)
                        time.sleep(2)
                        self.powered = True
            elif self.bit < 39:
                self.bad_SM += 1
                self.no_response = 0
            else:
                self.no_response = 0

    def temperature(self):
        return self.temp

    def humidity(self):
        return self.rhum

    def staleness(self):
        if self.tov is not None:
            return time.time() - self.tov
        else:
            return -999

    def trigger(self):
        if self.powered:
            if self.LED is not None:
                self.pi.write(self.LED, 1)
            self.pi.write(self.gpio, pigpio.LOW)
            time.sleep(0.017)
            self.pi.set_mode(self.gpio, pigpio.INPUT)
            self.pi.set_watchdog(self.gpio, 200)

    def cancel(self):
        self.pi.set_watchdog(self.gpio, 0)
        if self.cb is not None:
            self.cb.cancel()
            self.cb = None

# -----------------------------
# ROS2 Node
# -----------------------------
class DHT22Node(Node):
    def __init__(self):
        super().__init__('dht22_node')
        self.declare_parameter('gpio_pin', 27)
        self.declare_parameter('publish_rate', 0.5)

        gpio_pin = self.get_parameter('gpio_pin').value
        publish_rate = self.get_parameter('publish_rate').value

        # Connect to pigpiod
        self.pi = pigpio.pi()
        if not self.pi.connected:
            self.get_logger().error("Failed to connect to pigpiod")
            return

        self.get_logger().info(f"DHT22 node started on GPIO {gpio_pin} at {publish_rate} Hz")

        # Initialize sensor
        self.sensor = sensor(self.pi, gpio_pin)

        # Publishers
        self.temp_pub = self.create_publisher(Temperature, 'dht22/temperature', 10)
        self.humidity_pub = self.create_publisher(RelativeHumidity, 'dht22/humidity', 10)

        # Timer
        self.timer = self.create_timer(1.0 / publish_rate, self.timer_callback)
        self.last_trigger = 0

    def timer_callback(self):
        current_time = time.time()
        if current_time - self.last_trigger < 2.0:
            return  # DHT22 requires ~2s between reads

        self.sensor.trigger()
        self.last_trigger = current_time
        time.sleep(0.2)  # Wait for reading

        temperature = self.sensor.temperature()
        humidity = self.sensor.humidity()

        if temperature != -999 and humidity != -999:
            temp_msg = Temperature()
            temp_msg.header.stamp = self.get_clock().now().to_msg()
            temp_msg.header.frame_id = 'dht22_frame'
            temp_msg.temperature = temperature
            temp_msg.variance = 0.5
            self.temp_pub.publish(temp_msg)

            humidity_msg = RelativeHumidity()
            humidity_msg.header.stamp = self.get_clock().now().to_msg()
            humidity_msg.header.frame_id = 'dht22_frame'
            humidity_msg.relative_humidity = humidity / 100.0
            humidity_msg.variance = 0.02
            self.humidity_pub.publish(humidity_msg)

            self.get_logger().info(f"Temp: {temperature:.1f}°C  Humidity: {humidity:.1f}%")
        else:
            self.get_logger().warn("Failed to read DHT22 sensor")

    def destroy_node(self):
        if hasattr(self, 'sensor'):
            self.sensor.cancel()
        if hasattr(self, 'pi'):
            self.pi.stop()
        super().destroy_node()

# -----------------------------
# Main
# -----------------------------
def main(args=None):
    rclpy.init(args=args)
    node = DHT22Node()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()