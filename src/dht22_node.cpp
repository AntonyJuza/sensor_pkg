#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/temperature.hpp"
#include "sensor_msgs/msg/relative_humidity.hpp"
#include "sensor_pkg/dht22_driver.hpp"

class DHT22Node : public rclcpp::Node {
public:
    DHT22Node() : Node("dht22_node") {
        // Declare parameters
        this->declare_parameter("gpio_pin", 4);  // BCM GPIO 4
        this->declare_parameter("publish_rate", 0.5);  // Hz - DHT22 max is 0.5Hz
        
        int gpio_pin = this->get_parameter("gpio_pin").as_int();
        double rate = this->get_parameter("publish_rate").as_double();
        
        // Initialize driver
        dht22_ = std::make_unique<DHT22Driver>(gpio_pin);
        
        // Check if initialization was successful
        if (!dht22_) {
            RCLCPP_ERROR(this->get_logger(), "Failed to initialize DHT22 driver!");
            RCLCPP_ERROR(this->get_logger(), "Make sure pigpiod daemon is running: sudo systemctl start pigpiod");
            return;
        }
        
        // Create publishers
        temp_pub_ = this->create_publisher<sensor_msgs::msg::Temperature>(
            "dht22/temperature", 10);
        humidity_pub_ = this->create_publisher<sensor_msgs::msg::RelativeHumidity>(
            "dht22/humidity", 10);
        
        // Create timer
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / rate));
        timer_ = this->create_wall_timer(
            period, std::bind(&DHT22Node::timerCallback, this));
        
        RCLCPP_INFO(this->get_logger(), "DHT22 node started on GPIO %d at %.1f Hz", gpio_pin, rate);
        RCLCPP_INFO(this->get_logger(), "Connected to pigpiod daemon successfully");
    }

private:
    void timerCallback() {
        float temperature, humidity;
        
        if (dht22_->readSensor(temperature, humidity)) {
            // Publish temperature
            auto temp_msg = sensor_msgs::msg::Temperature();
            temp_msg.header.stamp = this->now();
            temp_msg.header.frame_id = "dht22_frame";
            temp_msg.temperature = temperature;
            temp_msg.variance = 0.25;  // ±0.5°C accuracy
            temp_pub_->publish(temp_msg);
            
            // Publish humidity
            auto humidity_msg = sensor_msgs::msg::RelativeHumidity();
            humidity_msg.header.stamp = this->now();
            humidity_msg.header.frame_id = "dht22_frame";
            humidity_msg.relative_humidity = humidity / 100.0;  // Convert to ratio
            humidity_msg.variance = 0.0004;  // ±2% accuracy
            humidity_pub_->publish(humidity_msg);
            
            RCLCPP_INFO(this->get_logger(), "Temp: %.1f°C, Humidity: %.1f%%", 
                        temperature, humidity);
        } else {
            RCLCPP_WARN(this->get_logger(), "Failed to read DHT22 sensor");
        }
    }
    
    std::unique_ptr<DHT22Driver> dht22_;
    rclcpp::Publisher<sensor_msgs::msg::Temperature>::SharedPtr temp_pub_;
    rclcpp::Publisher<sensor_msgs::msg::RelativeHumidity>::SharedPtr humidity_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DHT22Node>());
    rclcpp::shutdown();
    return 0;
}