#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/float32.hpp"
#include "sensor_pkg/flame_driver.hpp"

class FlameNode : public rclcpp::Node {
public:
    FlameNode() : Node("flame_node"), last_state_(false) {
        // Declare parameters
        this->declare_parameter("digital_gpio", 18);  // BCM GPIO 18
        this->declare_parameter("spi_channel", -1);   // MCP3008 channel (-1 = disabled)
        this->declare_parameter("publish_rate", 5.0);  // Hz
        
        int digital_gpio = this->get_parameter("digital_gpio").as_int();
        int spi_ch = this->get_parameter("spi_channel").as_int();
        double rate = this->get_parameter("publish_rate").as_double();
        
        // Initialize driver
        flame_ = std::make_unique<FlameDriver>(digital_gpio, spi_ch);
        
        // Create publishers
        detection_pub_ = this->create_publisher<std_msgs::msg::Bool>(
            "flame/detected", 10);
        
        if (spi_ch >= 0) {
            analog_pub_ = this->create_publisher<std_msgs::msg::Float32>(
                "flame/intensity", 10);
        }
        
        // Create timer
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / rate));
        timer_ = this->create_wall_timer(
            period, std::bind(&FlameNode::timerCallback, this));
        
        RCLCPP_INFO(this->get_logger(), 
                   "Flame sensor node started - GPIO: %d at %.1f Hz", 
                   digital_gpio, rate);
        if (spi_ch >= 0) {
            RCLCPP_INFO(this->get_logger(), "Analog SPI channel: %d", spi_ch);
        }
    }

private:
    void timerCallback() {
        bool flame_detected = flame_->flameDetected();
        
        // Publish detection status
        auto detection_msg = std_msgs::msg::Bool();
        detection_msg.data = flame_detected;
        detection_pub_->publish(detection_msg);
        
        // Read and publish analog value if available
        if (analog_pub_) {
            int analog_value = flame_->readAnalog();
            if (analog_value >= 0) {
                auto analog_msg = std_msgs::msg::Float32();
                analog_msg.data = static_cast<float>(analog_value);
                analog_pub_->publish(analog_msg);
            }
        }
        
        // Log state changes
        if (flame_detected != last_state_) {
            if (flame_detected) {
                RCLCPP_WARN(this->get_logger(), "FLAME DETECTED!");
            } else {
                RCLCPP_INFO(this->get_logger(), "Flame cleared");
            }
            last_state_ = flame_detected;
        } else {
            RCLCPP_DEBUG(this->get_logger(), "Flame status: %s", 
                        flame_detected ? "DETECTED" : "CLEAR");
        }
    }
    
    std::unique_ptr<FlameDriver> flame_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr detection_pub_;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr analog_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    bool last_state_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<FlameNode>());
    rclcpp::shutdown();
    return 0;
}