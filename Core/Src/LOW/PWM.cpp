#include "PWM.hpp"

PWM::PWM(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t timer_clock_hz)
    : htim(htim), channel(channel), timer_clock(timer_clock_hz),
      prescaler(0), period(65535), duty_percent(50.0f) {}

void PWM::apply_duty() {
    uint32_t duty = (uint32_t)((float)period * duty_percent / 100.0f);
    __HAL_TIM_SET_COMPARE(htim, channel, duty);
}

void PWM::start() {
    HAL_TIM_PWM_Start(htim, channel);
}

void PWM::stop() {
    HAL_TIM_PWM_Stop(htim, channel);
}

void PWM::set_duty_percent(float percent) {
    if (percent > 100.0f) percent = 100.0f;
    if (percent < 0.0f) percent = 0.0f;
    duty_percent = percent;
    apply_duty();
}

uint32_t PWM::calc_period(uint32_t freq_hz) {
    if (freq_hz == 0) return 65535;
    return (timer_clock / (prescaler + 1)) / freq_hz;
}

void PWM::set_freq(uint32_t freq_hz) {
    period = calc_period(freq_hz);
    if (period > 65535) period = 65535;
    if (period < 1) period = 1;
    __HAL_TIM_SET_AUTORELOAD(htim, period - 1);
    apply_duty();
}

uint32_t PWM::get_period() const {
    return period;
}
