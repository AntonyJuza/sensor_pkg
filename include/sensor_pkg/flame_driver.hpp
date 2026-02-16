#ifndef FLAME_DRIVER_HPP
#define FLAME_DRIVER_HPP

#include <cstdint>

class FlameDriver {
public:
    FlameDriver(int digital_pin, int analog_pin = -1);
    ~FlameDriver();
    
    bool flameDetected();  // Digital reading
    int readAnalog();      // Analog reading (0-1023) if available
    
private:
    int digital_pin_;
    int analog_pin_;
    int gpio_handle_;
    int readMCP3008(int channel);
};

#endif // FLAME_DRIVER_HPP
