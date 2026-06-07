#ifndef LOW_DIGITAL_OUTPUT_HPP
#define LOW_DIGITAL_OUTPUT_HPP

#include "stm32f1xx_hal.h"

class DigitalOutput {
public:
    DigitalOutput(GPIO_TypeDef *port, uint16_t pin);
    void write(bool state);
    void toggle();
    bool read() const;
private:
    GPIO_TypeDef *port;
    uint16_t pin;
};

#endif
