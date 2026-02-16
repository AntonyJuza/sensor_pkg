#include "sensor_pkg/mq2_driver.hpp"
#include <pigpio.h>
#include <cmath>
#include <unistd.h>

// SPI settings for MCP3008
#define SPI_SPEED 1000000  // 1 MHz

MQ2Driver::MQ2Driver(int spi_channel, int digital_gpio) 
    : spi_channel_(spi_channel), digital_gpio_(digital_gpio), 
      spi_handle_(-1), initialized_(false), ro_(10.0) {
    
    // Initialize pigpio
    if (gpioInitialise() < 0) {
        return;
    }
    
    // Setup digital GPIO pin as input
    gpioSetMode(digital_gpio_, PI_INPUT);
    
    // Open SPI for MCP3008 ADC
    // SPI channel 0, baud rate 1MHz, flags: mode 0
    spi_handle_ = spiOpen(0, SPI_SPEED, 0);
    if (spi_handle_ < 0) {
        return;
    }
    
    initialized_ = true;
    
    // Calibrate sensor
    calibrate();
}

MQ2Driver::~MQ2Driver() {
    if (spi_handle_ >= 0) {
        spiClose(spi_handle_);
    }
    if (initialized_) {
        gpioTerminate();
    }
}

void MQ2Driver::calibrate() {
    if (!initialized_) return;
    
    // Read sensor resistance in clean air (average of 50 readings)
    float rs_air = 0.0;
    for (int i = 0; i < 50; i++) {
        int adc = readMCP3008(spi_channel_);
        float sensor_volt = (adc / 1024.0) * 3.3;
        float rs = (3.3 - sensor_volt) / sensor_volt * 10.0;  // RL = 10k
        rs_air += rs;
        gpioDelay(50000);  // 50ms delay
    }
    ro_ = rs_air / 50.0;
    ro_ = ro_ / 9.83;  // Ratio in clean air for MQ2
}

int MQ2Driver::readMCP3008(int channel) {
    if (spi_handle_ < 0 || channel < 0 || channel > 7) {
        return 0;
    }
    
    // MCP3008 protocol: start bit, single-ended mode, channel selection
    unsigned char buffer[3];
    buffer[0] = 1;  // Start bit
    buffer[1] = (8 + channel) << 4;  // Single-ended channel selection
    buffer[2] = 0;
    
    spiXfer(spi_handle_, (char*)buffer, (char*)buffer, 3);
    
    // Extract 10-bit ADC value
    return ((buffer[1] & 3) << 8) + buffer[2];
}

int MQ2Driver::readAnalog() {
    return readMCP3008(spi_channel_);
}

bool MQ2Driver::readDigital() {
    if (!initialized_) return false;
    return gpioRead(digital_gpio_) == PI_LOW;  // Active LOW
}

float MQ2Driver::getConcentrationPPM() {
    if (!initialized_) return 0.0;
    
    int adc = readMCP3008(spi_channel_);
    
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