#include "sensor_pkg/flame_driver.hpp"
#include <pigpiod_if2.h>
#include <iostream>

#define SPI_CHANNEL 0
#define SPI_SPEED 1000000

FlameDriver::FlameDriver(int digital_pin, int analog_pin) 
    : digital_pin_(digital_pin), analog_pin_(analog_pin), gpio_handle_(-1) {
    
    // Connect to pigpiod daemon
    gpio_handle_ = pigpio_start(NULL, NULL);
    if (gpio_handle_ < 0) {
        std::cerr << "Failed to connect to pigpio daemon! Try 'sudo pigpiod'" << std::endl;
        return;
    }
    
    gpioSetMode(gpio_handle_, digital_pin_, PI_OUTPUT);
    
    // Initialize SPI if analog pin is specified
    if (analog_pin_ >= 0) {
        spiOpen(gpio_handle_, SPI_CHANNEL, SPI_SPEED, 0);
    }
}

FlameDriver::~FlameDriver() {
}

bool FlameDriver::flameDetected() {
    // Most flame sensors are active LOW (LOW = flame detected)
    return gpioRead(gpio_handle_, digital_pin_) == 0;  // LOW = 0
}

int FlameDriver::readMCP3008(int channel) {
    if (channel < 0) return 0;
    
    unsigned char buffer[3];
    buffer[0] = 1;  // Start bit
    buffer[1] = (8 + channel) << 4;  // Single-ended channel selection
    buffer[2] = 0;
    
    spiXfer(gpio_handle_, SPI_CHANNEL, (char*)buffer, (char*)buffer, 3);
    
    return ((buffer[1] & 3) << 8) + buffer[2];
}

int FlameDriver::readAnalog() {
    if (analog_pin_ < 0) {
        return -1;  // Analog not configured
    }
    return readMCP3008(analog_pin_);
}