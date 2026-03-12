#ifndef DHT22_DRIVER_HPP
#define DHT22_DRIVER_HPP

#include <cstdint>

class DHT22Driver {
public:
    DHT22Driver(int gpio_pin);
    ~DHT22Driver();
    
    bool readSensor(float &temperature, float &humidity);
    
private:
    int gpio_pin_;
    int pi_;  // pigpio instance
    bool initialized_;
    int readData(uint8_t data[5]);
};

#endif // DHT22_DRIVER_HPP