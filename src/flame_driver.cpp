#include "sensor_pkg/flame_driver.hpp"
#include <pigpio.h>

#define SPI_SPEED 1000000  // 1 MHz

FlameDriver::FlameDriver(int digital_gpio, int spi_channel) 
    : digital_gpio_(digital_gpio), spi_channel_(spi_channel), 
      spi_handle_(-1), initialized_(false) {
    
    // Initialize pigpio
    if (gpioInitialise() < 0) {
        return;
    }
    
    // Setup digital GPIO pin as input
    gpioSetMode(digital_gpio_, PI_INPUT);
    
    // Initialize SPI if analog channel is specified
    if (spi_channel_ >= 0) {
        spi_handle_ = spiOpen(0, SPI_SPEED, 0);
        if (spi_handle_ < 0) {
            return;
        }
    }
    
    initialized_ = true;
}

FlameDriver::~FlameDriver() {
    if (spi_handle_ >= 0) {
        spiClose(spi_handle_);
    }
    if (initialized_) {
        gpioTerminate();
    }
}

bool FlameDriver::flameDetected() {
    if (!initialized_) return false;
    // Most flame sensors are active LOW (LOW = flame detected)
    return gpioRead(digital_gpio_) == PI_LOW;
}

int FlameDriver::readMCP3008(int channel) {
    if (spi_handle_ < 0 || channel < 0 || channel > 7) {
        return -1;
    }
    
    // MCP3008 protocol
    unsigned char buffer[3];
    buffer[0] = 1;  // Start bit
    buffer[1] = (8 + channel) << 4;  // Single-ended channel selection
    buffer[2] = 0;
    
    spiXfer(spi_handle_, (char*)buffer, (char*)buffer, 3);
    
    // Extract 10-bit ADC value
    return ((buffer[1] & 3) << 8) + buffer[2];
}

int FlameDriver::readAnalog() {
    if (spi_channel_ < 0) {
        return -1;  // Analog not configured
    }
    return readMCP3008(spi_channel_);
}