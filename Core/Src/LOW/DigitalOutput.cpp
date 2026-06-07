#include "DigitalOutput.hpp"

DigitalOutput::DigitalOutput(GPIO_TypeDef *port, uint16_t pin)
    : port(port), pin(pin) {}

void DigitalOutput::write(bool state) {
    HAL_GPIO_WritePin(port, pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void DigitalOutput::toggle() {
    HAL_GPIO_TogglePin(port, pin);
}

bool DigitalOutput::read() const {
    return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET;
}
