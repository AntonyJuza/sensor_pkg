#include "sensor_pkg/mq2_driver.hpp"
#include <pigpiod_if2.h>
#include <cmath>
#include <iostream>

// MCP3008 SPI settings
#define SPI_CHANNEL 0
#define SPI_SPEED 1000000

MQ2Driver::MQ2Driver(int analog_pin, int digital_pin) 
    : analog_pin_(analog_pin), digital_pin_(digital_pin), ro_(10.0), gpio_handle_(-1) {
    
    // Connect to pigpiod daemon
    gpio_handle_ = pigpio_start(NULL, NULL);
    if (gpio_handle_ < 0) {
        std::cerr << "Failed to connect to pigpio daemon! Try 'sudo pigpiod'" << std::endl;
        return;
    }
    
    // Setup digital pin
    gpioSetMode(gpio_handle_, digital_pin_, PI_INPUT);
    
    // Initialize SPI for analog reading (MCP3008)
    spiOpen(gpio_handle_, SPI_CHANNEL, SPI_SPEED, 0);
    
    // Calibrate sensor
    calibrate();
}

MQ2Driver::~MQ2Driver() {
}

void MQ2Driver::calibrate() {
    // Read sensor resistance in clean air (average of 50 readings)
    float rs_air = 0.0;
    for (int i = 0; i < 50; i++) {
        int adc = readMCP3008(analog_pin_);
        float sensor_volt = (adc / 1024.0) * 3.3;
        float rs = (3.3 - sensor_volt) / sensor_volt * 10.0;  // RL = 10k
        rs_air += rs;
        gpioDelay(gpio_handle_, 50000);  // 50ms (50000 microseconds)
    }
    ro_ = rs_air / 50.0;
    ro_ = ro_ / 9.83;  // Ratio in clean air for MQ2
}

int MQ2Driver::readMCP3008(int channel) {
    unsigned char buffer[3];
    buffer[0] = 1;  // Start bit
    buffer[1] = (8 + channel) << 4;  // Single-ended channel selection
    buffer[2] = 0;
    
    spiXfer(gpio_handle_, SPI_CHANNEL, (char*)buffer, (char*)buffer, 3);
    
    return ((buffer[1] & 3) << 8) + buffer[2];
}

int MQ2Driver::readAnalog() {
    return readMCP3008(analog_pin_);
}

bool MQ2Driver::readDigital() {
    return gpioRead(gpio_handle_, digital_pin_) == 0;  // Active LOW = 0
}

float MQ2Driver::getConcentrationPPM() {
    int adc = readMCP3008(analog_pin_);
    
    if (adc == 0) return 0.0;
    
    // Calculate sensor resistance
    float sensor_volt = (adc / 1024.0) * 3.3;
    if (sensor_volt == 0) return 0.0;
    
    float rs = (3.3 - sensor_volt) / sensor_volt * 10.0;  // RL = 10k
    float ratio = rs / ro_;
    
    // MQ2 curve approximation for LPG/Smoke
    // PPM = 613.9 * ratio^(-2.074) (from datasheet)
    float ppm = 613.9 * pow(ratio, -2.074);
    
    return ppm;
}