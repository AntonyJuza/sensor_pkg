#ifndef FLAME_DRIVER_HPP
#define FLAME_DRIVER_HPP

#include <cstdint>

class FlameDriver {
public:
    FlameDriver(int digital_gpio, int spi_channel = -1);
    ~FlameDriver();
    
    bool flameDetected();  // Digital reading
    int readAnalog();      // Analog reading (0-1023) if available
    
private:
    int digital_gpio_;
    int spi_channel_;
    int pi_;               // pigpio daemon handle
    int spi_handle_;
    bool initialized_;
    int readMCP3008(int channel);
};

#endif // FLAME_DRIVER_HPP