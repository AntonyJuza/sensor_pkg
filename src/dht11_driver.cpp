#include "sensor_pkg/dht11_driver.hpp"
#include <pigpiod_if2.h>
#include <cstring>
#include <thread>
#include <chrono>
#include <iostream>

DHT11Driver::DHT11Driver(int pin) : pin_(pin), gpio_handle_(-1) {
    // Connect to pigpiod daemon
    gpio_handle_ = pigpio_start(NULL, NULL);
    if (gpio_handle_ < 0) {
        std::cerr << "Failed to connect to pigpio daemon! Try 'sudo pigpiod'" << std::endl;
        return;
    }
    
    gpioSetMode(gpio_handle_, pin_, 1);  // OUTPUT = 1
    gpioWrite(gpio_handle_, pin_, 1);  // HIGH = 1
}

DHT11Driver::~DHT11Driver() {
}

void DHT11Driver::delayMicroseconds(int microseconds) {
    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

bool DHT11Driver::readSensor(float &temperature, float &humidity) {
    uint8_t data[5] = {0, 0, 0, 0, 0};
    
    if (readData(data) != 0) {
        return false;
    }
    
    // Verify checksum
    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        return false;
    }
    
    // DHT11 returns integer values
    humidity = static_cast<float>(data[0]) + static_cast<float>(data[1]) * 0.1f;
    temperature = static_cast<float>(data[2]) + static_cast<float>(data[3]) * 0.1f;
    
    return true;
}

int DHT11Driver::readData(uint8_t data[5]) {
    uint8_t laststate = 1;  // START with HIGH
    uint8_t counter = 0;
    uint8_t j = 0, i;

    // Reset data array
    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    // Send start signal
    gpioSetMode(gpio_handle_, pin_, 1);  // OUTPUT = 1
    gpioWrite(gpio_handle_, pin_, 0);  // LOW = 0
    gpioDelay(18000);  // At least 18ms (18000 microseconds) - direct mode function
    gpioWrite(gpio_handle_, pin_, 1);  // HIGH = 1
    delayMicroseconds(40);
    
    // Switch to input mode to read data
    gpioSetMode(gpio_handle_, pin_, 0);  // INPUT = 0

    // Wait for sensor response and read 40 bits (5 bytes)
    for (i = 0; i < 85; i++) {
        counter = 0;
        while (gpioRead(gpio_handle_, pin_) == laststate) {
            counter++;
            delayMicroseconds(1);
            if (counter == 255) {
                break;
            }
        }
        laststate = gpioRead(gpio_handle_, pin_);

        if (counter == 255) break;

        // Ignore first 3 transitions (sensor response)
        if ((i >= 4) && (i % 2 == 0)) {
            // Shift data and add new bit
            data[j / 8] <<= 1;
            if (counter > 16)  // High bit duration > low bit duration
                data[j / 8] |= 1;
            j++;
        }
    }

    // Check if we read 40 bits (5 bytes)
    if (j >= 40) {
        return 0;  // Success
    }
    
    return -1;  // Error
}