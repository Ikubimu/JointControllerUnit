#ifndef LOW_PWM_HPP
#define LOW_PWM_HPP

#include "stm32f1xx_hal.h"

class PWM {
public:
    PWM(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t timer_clock_hz);
    void start();
    void stop();
    void set_duty_percent(float percent);
    void set_freq(uint32_t freq_hz);
    uint32_t get_period() const;
private:
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    uint32_t timer_clock;
    uint32_t prescaler;
    uint32_t period;
    float duty_percent;
    uint32_t calc_period(uint32_t freq_hz);
    void apply_duty();
};

#endif
