#include "sensor_pkg/dht11_driver.hpp"
#include <pigpiod_if2.h>
#include <cstring>
#include <unistd.h>

DHT11Driver::DHT11Driver(int gpio_pin) : gpio_pin_(gpio_pin), pi_(-1), initialized_(false) {
    // Connect to pigpio daemon (localhost, default port)
    pi_ = pigpio_start(NULL, NULL);
    if (pi_ < 0) {
        // Failed to connect to daemon
        return;
    }
    
    // Set pin as output initially
    set_mode(pi_, gpio_pin_, PI_OUTPUT);
    gpio_write(pi_, gpio_pin_, 1);
    initialized_ = true;
}

DHT11Driver::~DHT11Driver() {
    if (pi_ >= 0) {
        pigpio_stop(pi_);
    }
}

bool DHT11Driver::readSensor(float &temperature, float &humidity) {
    if (!initialized_ || pi_ < 0) {
        return false;
    }
    
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
    if (pi_ < 0) return -1;
    
    uint8_t laststate = PI_HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    
    // Reset data array
    data[0] = data[1] = data[2] = data[3] = data[4] = 0;
    
    // Send start signal
    set_mode(pi_, gpio_pin_, PI_OUTPUT);
    gpio_write(pi_, gpio_pin_, PI_LOW);
    time_sleep(0.018);  // 18ms delay
    gpio_write(pi_, gpio_pin_, PI_HIGH);
    time_sleep(0.00004);  // 40us delay
    
    // Switch to input mode to read data
    set_mode(pi_, gpio_pin_, PI_INPUT);
    
    // Wait for sensor response and read 40 bits (5 bytes)
    for (i = 0; i < 85; i++) {
        counter = 0;
        while (gpio_read(pi_, gpio_pin_) == laststate) {
            counter++;
            time_sleep(0.000001);  // 1us delay
            if (counter == 255) {
                break;
            }
        }
        laststate = gpio_read(pi_, gpio_pin_);
        
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