#ifndef DHT11_DRIVER_HPP
#define DHT11_DRIVER_HPP

#include <cstdint>

class DHT11Driver {
public:
    DHT11Driver(int pin);
    ~DHT11Driver();
    
    bool readSensor(float &temperature, float &humidity);
    
private:
    int pin_;
    int gpio_handle_;
    int readData(uint8_t data[5]);
    void delayMicroseconds(int microseconds);
};

#endif // DHT11_DRIVER_HPP
