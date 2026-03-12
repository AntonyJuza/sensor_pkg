#include "sensor_pkg/dht22_driver.hpp"
#include <pigpiod_if2.h>
#include <cstring>
#include <unistd.h>

DHT22Driver::DHT22Driver(int gpio_pin) : gpio_pin_(gpio_pin), pi_(-1), initialized_(false) {
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

DHT22Driver::~DHT22Driver() {
    if (pi_ >= 0) {
        pigpio_stop(pi_);
    }
}

bool DHT22Driver::readSensor(float &temperature, float &humidity) {
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
    
    // DHT22 returns values with decimal precision
    // Humidity: data[0] is integer part, data[1] is decimal part
    humidity = ((data[0] << 8) + data[1]) * 0.1f;
    
    // Temperature: data[2] is integer part (bit 7 is sign), data[3] is decimal part
    int temp_raw = ((data[2] & 0x7F) << 8) + data[3];
    temperature = temp_raw * 0.1f;
    
    // Check sign bit for negative temperature
    if (data[2] & 0x80) {
        temperature = -temperature;
    }
    
    return true;
}

int DHT22Driver::readData(uint8_t data[5]) {
    if (pi_ < 0) return -1;
    
    uint32_t start_tick, current_tick;
    
    // Reset data array
    data[0] = data[1] = data[2] = data[3] = data[4] = 0;
    
    // Send start signal - DHT22 needs at least 1ms LOW
    set_mode(pi_, gpio_pin_, PI_OUTPUT);
    gpio_write(pi_, gpio_pin_, PI_LOW);
    time_sleep(0.001);  // 1ms for DHT22 (DHT11 needs 18ms)
    
    gpio_write(pi_, gpio_pin_, PI_HIGH);
    time_sleep(0.000030);  // 30μs
    
    // Switch to input mode
    set_mode(pi_, gpio_pin_, PI_INPUT);
    
    // Wait for sensor to pull low (response signal)
    start_tick = get_current_tick(pi_);
    while (gpio_read(pi_, gpio_pin_) == PI_HIGH) {
        current_tick = get_current_tick(pi_);
        if ((current_tick - start_tick) > 100) {  // 100μs timeout
            return -1;  // Timeout - sensor not responding
        }
    }
    
    // Wait for sensor to pull high (should be ~80μs)
    start_tick = get_current_tick(pi_);
    while (gpio_read(pi_, gpio_pin_) == PI_LOW) {
        current_tick = get_current_tick(pi_);
        if ((current_tick - start_tick) > 100) {
            return -1;
        }
    }
    
    // Wait for sensor to pull low again (should be ~80μs)
    start_tick = get_current_tick(pi_);
    while (gpio_read(pi_, gpio_pin_) == PI_HIGH) {
        current_tick = get_current_tick(pi_);
        if ((current_tick - start_tick) > 100) {
            return -1;
        }
    }
    
    // Read 40 bits of data
    for (int i = 0; i < 40; i++) {
        // Wait for bit start (goes HIGH after ~50μs LOW)
        start_tick = get_current_tick(pi_);
        while (gpio_read(pi_, gpio_pin_) == PI_LOW) {
            current_tick = get_current_tick(pi_);
            if ((current_tick - start_tick) > 100) {
                return -1;
            }
        }
        
        // Measure HIGH duration to determine bit value
        // DHT22: 26-28μs = 0, 70μs = 1
        uint32_t high_start = get_current_tick(pi_);
        while (gpio_read(pi_, gpio_pin_) == PI_HIGH) {
            current_tick = get_current_tick(pi_);
            if ((current_tick - high_start) > 100) {
                break;
            }
        }
        uint32_t high_duration = get_current_tick(pi_) - high_start;
        
        // Shift and add bit
        data[i / 8] <<= 1;
        if (high_duration > 40) {  // If HIGH > 40μs, bit is 1
            data[i / 8] |= 1;
        }
    }
    
    // Check if we read valid data (at least some non-zero values)
    if (data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 0) {
        return -1;  // All zeros is invalid
    }
    
    return 0;  // Success
}