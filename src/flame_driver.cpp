#include "sensor_pkg/flame_driver.hpp"
#include <pigpiod_if2.h>

#define SPI_SPEED 1000000  // 1 MHz

FlameDriver::FlameDriver(int digital_gpio, int spi_channel) 
    : digital_gpio_(digital_gpio), spi_channel_(spi_channel), 
      pi_(-1), spi_handle_(-1), initialized_(false) {
    
    // Connect to pigpio daemon
    pi_ = pigpio_start(NULL, NULL);
    if (pi_ < 0) {
        return;
    }
    
    // Setup digital GPIO pin as input
    set_mode(pi_, digital_gpio_, PI_INPUT);
    
    // Initialize SPI if analog channel is specified
    if (spi_channel_ >= 0) {
        spi_handle_ = spi_open(pi_, 0, SPI_SPEED, 0);
        if (spi_handle_ < 0) {
            pigpio_stop(pi_);
            pi_ = -1;
            return;
        }
    }
    
    initialized_ = true;
}

FlameDriver::~FlameDriver() {
    if (spi_handle_ >= 0) {
        spi_close(pi_, spi_handle_);
    }
    if (pi_ >= 0) {
        pigpio_stop(pi_);
    }
}

bool FlameDriver::flameDetected() {
    if (!initialized_ || pi_ < 0) return false;
    
    int result = gpio_read(pi_, digital_gpio_);
    
    // Most flame sensors are active LOW (LOW = flame detected)
    return result == PI_LOW;
}

int FlameDriver::readMCP3008(int channel) {
    if (pi_ < 0 || spi_handle_ < 0 || channel < 0 || channel > 7) {
        return -1;
    }
    
    // MCP3008 protocol
    char buffer[3];
    buffer[0] = 1;  // Start bit
    buffer[1] = (8 + channel) << 4;  // Single-ended channel selection
    buffer[2] = 0;
    
    int count = spi_xfer(pi_, spi_handle_, buffer, buffer, 3);
    if (count < 0) return -1;
    
    // Extract 10-bit ADC value
    return ((buffer[1] & 3) << 8) + buffer[2];
}

int FlameDriver::readAnalog() {
    if (spi_channel_ < 0) {
        return -1;  // Analog not configured
    }
    return readMCP3008(spi_channel_);
}