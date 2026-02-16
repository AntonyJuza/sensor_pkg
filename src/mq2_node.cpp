#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float32.hpp"
#include "std_msgs/msg/bool.hpp"
#include "sensor_pkg/mq2_driver.hpp"

class MQ2Node : public rclcpp::Node {
public:
    MQ2Node() : Node("mq2_node") {
        // Declare parameters
        this->declare_parameter("analog_channel", 0);  // MCP3008 channel
        this->declare_parameter("digital_pin", 0);     // WiringPi pin for digital out
        this->declare_parameter("publish_rate", 2.0);  // Hz
        this->declare_parameter("gas_threshold", 300.0);  // PPM threshold
        
        int analog_ch = this->get_parameter("analog_channel").as_int();
        int digital_pin = this->get_parameter("digital_pin").as_int();
        double rate = this->get_parameter("publish_rate").as_double();
        gas_threshold_ = this->get_parameter("gas_threshold").as_double();
        
        // Initialize driver
        mq2_ = std::make_unique<MQ2Driver>(analog_ch, digital_pin);
        
        // Create publishers
        ppm_pub_ = this->create_publisher<std_msgs::msg::Float32>(
            "mq2/gas_ppm", 10);
        digital_pub_ = this->create_publisher<std_msgs::msg::Bool>(
            "mq2/gas_detected", 10);
        adc_pub_ = this->create_publisher<std_msgs::msg::Float32>(
            "mq2/adc_value", 10);
        
        // Create timer
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / rate));
        timer_ = this->create_wall_timer(
            period, std::bind(&MQ2Node::timerCallback, this));
        
        RCLCPP_INFO(this->get_logger(), 
                   "MQ2 node started - Analog CH: %d, Digital Pin: %d at %.1f Hz", 
                   analog_ch, digital_pin, rate);
        RCLCPP_INFO(this->get_logger(), "Gas threshold: %.1f PPM", gas_threshold_);
    }

private:
    void timerCallback() {
        // Read analog value
        int adc_value = mq2_->readAnalog();
        
        // Read digital value
        bool digital_detected = mq2_->readDigital();
        
        // Get PPM concentration
        float ppm = mq2_->getConcentrationPPM();
        
        // Publish ADC value
        auto adc_msg = std_msgs::msg::Float32();
        adc_msg.data = static_cast<float>(adc_value);
        adc_pub_->publish(adc_msg);
        
        // Publish PPM
        auto ppm_msg = std_msgs::msg::Float32();
        ppm_msg.data = ppm;
        ppm_pub_->publish(ppm_msg);
        
        // Publish digital detection
        auto digital_msg = std_msgs::msg::Bool();
        digital_msg.data = digital_detected;
        digital_pub_->publish(digital_msg);
        
        // Log warnings if gas detected
        if (ppm > gas_threshold_) {
            RCLCPP_WARN(this->get_logger(), 
                       "Gas concentration HIGH: %.1f PPM (ADC: %d)", ppm, adc_value);
        } else {
            RCLCPP_DEBUG(this->get_logger(), 
                        "Gas: %.1f PPM, ADC: %d, Digital: %s", 
                        ppm, adc_value, digital_detected ? "DETECTED" : "CLEAR");
        }
    }
    
    std::unique_ptr<MQ2Driver> mq2_;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr ppm_pub_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr digital_pub_;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr adc_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    double gas_threshold_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MQ2Node>());
    rclcpp::shutdown();
    return 0;
}