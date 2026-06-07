#ifndef LOW_ADC_HPP
#define LOW_ADC_HPP

#include "stm32f1xx_hal.h"

class ADC {
public:
    ADC(ADC_HandleTypeDef *hadc);
    uint32_t read_channel(uint32_t channel);
private:
    ADC_HandleTypeDef *hadc;
};

#endif
