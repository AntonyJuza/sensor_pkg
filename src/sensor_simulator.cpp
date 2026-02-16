#include <memory>
#include <random>
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/temperature.hpp"
#include "sensor_msgs/msg/relative_humidity.hpp"
#include "std_msgs/msg/float32.hpp"
#include "std_msgs/msg/bool.hpp"

class SensorSimulatorNode : public rclcpp::Node {
public:
    SensorSimulatorNode() : Node("sensor_simulator"), 
        gen_(rd_()), 
        temp_dist_(20.0, 5.0),
        humidity_dist_(40.0, 15.0),
        gas_dist_(100.0, 50.0) {
        
        // Publishers for DHT11
        temp_pub_ = this->create_publisher<sensor_msgs::msg::Temperature>(
            "dht11/temperature", 10);
        humidity_pub_ = this->create_publisher<sensor_msgs::msg::RelativeHumidity>(
            "dht11/humidity", 10);
        
        // Publishers for MQ2
        gas_ppm_pub_ = this->create_publisher<std_msgs::msg::Float32>(
            "mq2/gas_ppm", 10);
        gas_detected_pub_ = this->create_publisher<std_msgs::msg::Bool>(
            "mq2/gas_detected", 10);
        
        // Publishers for Flame
        flame_detected_pub_ = this->create_publisher<std_msgs::msg::Bool>(
            "flame/detected", 10);
        
        // Timer
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&SensorSimulatorNode::timerCallback, this));
        
        RCLCPP_INFO(this->get_logger(), "Sensor simulator started (no hardware required)");
    }

private:
    void timerCallback() {
        auto now = this->now();
        
        // Simulate DHT11
        float temperature = temp_dist_(gen_);
        float humidity = humidity_dist_(gen_);
        
        auto temp_msg = sensor_msgs::msg::Temperature();
        temp_msg.header.stamp = now;
        temp_msg.header.frame_id = "dht11_frame";
        temp_msg.temperature = temperature;
        temp_msg.variance = 0.5;
        temp_pub_->publish(temp_msg);
        
        auto humidity_msg = sensor_msgs::msg::RelativeHumidity();
        humidity_msg.header.stamp = now;
        humidity_msg.header.frame_id = "dht11_frame";
        humidity_msg.relative_humidity = humidity / 100.0;
        humidity_msg.variance = 0.05;
        humidity_pub_->publish(humidity_msg);
        
        // Simulate MQ2
        float gas_ppm = std::max(0.0f, static_cast<float>(gas_dist_(gen_)));
        bool gas_detected = gas_ppm > 300.0;
        
        auto gas_msg = std_msgs::msg::Float32();
        gas_msg.data = gas_ppm;
        gas_ppm_pub_->publish(gas_msg);
        
        auto gas_detected_msg = std_msgs::msg::Bool();
        gas_detected_msg.data = gas_detected;
        gas_detected_pub_->publish(gas_detected_msg);
        
        // Simulate Flame (random with low probability)
        bool flame = (rand() % 100) < 5;  // 5% chance
        
        auto flame_msg = std_msgs::msg::Bool();
        flame_msg.data = flame;
        flame_detected_pub_->publish(flame_msg);
        
        RCLCPP_INFO(this->get_logger(), 
                   "Simulated - Temp: %.1f°C, Humidity: %.1f%%, Gas: %.1f PPM, Flame: %s",
                   temperature, humidity, gas_ppm, flame ? "YES" : "NO");
    }
    
    std::random_device rd_;
    std::mt19937 gen_;
    std::normal_distribution<> temp_dist_;
    std::normal_distribution<> humidity_dist_;
    std::normal_distribution<> gas_dist_;
    
    rclcpp::Publisher<sensor_msgs::msg::Temperature>::SharedPtr temp_pub_;
    rclcpp::Publisher<sensor_msgs::msg::RelativeHumidity>::SharedPtr humidity_pub_;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr gas_ppm_pub_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr gas_detected_pub_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr flame_detected_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SensorSimulatorNode>());
    rclcpp::shutdown();
    return 0;
}