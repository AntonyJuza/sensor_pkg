#ifndef MQ2_DRIVER_HPP
#define MQ2_DRIVER_HPP

#include <cstdint>

class MQ2Driver {
public:
    MQ2Driver(int analog_pin, int digital_pin);
    ~MQ2Driver();
    
    int readAnalog();  // Returns ADC value (0-1023)
    bool readDigital();  // Returns true if gas detected
    float getConcentrationPPM();  // Estimated gas concentration
    
private:
    int analog_pin_;
    int digital_pin_;
    int gpio_handle_;
    int readMCP3008(int channel);  // For reading analog via MCP3008 ADC
    float ro_;  // Sensor resistance in clean air
    void calibrate();
};

#endif // MQ2_DRIVER_HPP
