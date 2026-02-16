#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/temperature.hpp"
#include "sensor_msgs/msg/relative_humidity.hpp"
#include "sensor_pkg/dht11_driver.hpp"

class DHT11Node : public rclcpp::Node {
public:
    DHT11Node() : Node("dht11_node") {
        // Declare parameters
        this->declare_parameter("pin", 7);  // WiringPi pin 7 (GPIO 4)
        this->declare_parameter("publish_rate", 1.0);  // Hz
        
        int pin = this->get_parameter("pin").as_int();
        double rate = this->get_parameter("publish_rate").as_double();
        
        // Initialize driver
        dht11_ = std::make_unique<DHT11Driver>(pin);
        
        // Create publishers
        temp_pub_ = this->create_publisher<sensor_msgs::msg::Temperature>(
            "dht11/temperature", 10);
        humidity_pub_ = this->create_publisher<sensor_msgs::msg::RelativeHumidity>(
            "dht11/humidity", 10);
        
        // Create timer
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / rate));
        timer_ = this->create_wall_timer(
            period, std::bind(&DHT11Node::timerCallback, this));
        
        RCLCPP_INFO(this->get_logger(), "DHT11 node started on pin %d at %.1f Hz", pin, rate);
    }

private:
    void timerCallback() {
        float temperature, humidity;
        
        if (dht11_->readSensor(temperature, humidity)) {
            // Publish temperature
            auto temp_msg = sensor_msgs::msg::Temperature();
            temp_msg.header.stamp = this->now();
            temp_msg.header.frame_id = "dht11_frame";
            temp_msg.temperature = temperature;
            temp_msg.variance = 0.5;  // ±0.5°C accuracy
            temp_pub_->publish(temp_msg);
            
            // Publish humidity
            auto humidity_msg = sensor_msgs::msg::RelativeHumidity();
            humidity_msg.header.stamp = this->now();
            humidity_msg.header.frame_id = "dht11_frame";
            humidity_msg.relative_humidity = humidity / 100.0;  // Convert to ratio
            humidity_msg.variance = 0.05;  // ±5% accuracy
            humidity_pub_->publish(humidity_msg);
            
            RCLCPP_DEBUG(this->get_logger(), "Temp: %.1f°C, Humidity: %.1f%%", 
                        temperature, humidity);
        } else {
            RCLCPP_WARN(this->get_logger(), "Failed to read DHT11 sensor");
        }
    }
    
    std::unique_ptr<DHT11Driver> dht11_;
    rclcpp::Publisher<sensor_msgs::msg::Temperature>::SharedPtr temp_pub_;
    rclcpp::Publisher<sensor_msgs::msg::RelativeHumidity>::SharedPtr humidity_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DHT11Node>());
    rclcpp::shutdown();
    return 0;
}