#ifndef DHT11_DRIVER_HPP
#define DHT11_DRIVER_HPP

#include <cstdint>

class DHT11Driver {
public:
    DHT11Driver(int gpio_pin);
    ~DHT11Driver();
    
    bool readSensor(float &temperature, float &humidity);
    
private:
    int gpio_pin_;
    int pi_;  // pigpio instance
    bool initialized_;
    int readData(uint8_t data[5]);
};

#endif // DHT11_DRIVER_HPP