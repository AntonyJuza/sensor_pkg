#ifndef MQ2_DRIVER_HPP
#define MQ2_DRIVER_HPP

#include <cstdint>

class MQ2Driver {
public:
    MQ2Driver(int spi_channel, int digital_gpio);
    ~MQ2Driver();
    
    int readAnalog();  // Returns ADC value (0-1023)
    bool readDigital();  // Returns true if gas detected
    float getConcentrationPPM();  // Estimated gas concentration
    
private:
    int spi_channel_;      // MCP3008 channel (0-7)
    int digital_gpio_;     // GPIO pin for digital output
    int spi_handle_;       // pigpio SPI handle
    bool initialized_;
    int readMCP3008(int channel);
    float ro_;  // Sensor resistance in clean air
    void calibrate();
};

#endif // MQ2_DRIVER_HPP