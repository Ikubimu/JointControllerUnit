#include "Motor.hpp"

static const uint8_t microstep_table[5][3] = {
    {0, 0, 0},  // MICROSTEP_1  (full)
    {1, 0, 0},  // MICROSTEP_2  (half)
    {0, 1, 0},  // MICROSTEP_4  (quarter)
    {1, 1, 0},  // MICROSTEP_8  (eighth)
    {1, 1, 1},  // MICROSTEP_16 (sixteenth)
};

Motor::Motor(PWM &pwm, ADC &adc, uint32_t adc_channel,
             DigitalOutput &ms1, DigitalOutput &ms2, DigitalOutput &ms3)
    : pwm(pwm), adc(adc), adc_channel(adc_channel),
      ms1(ms1), ms2(ms2), ms3(ms3),
      current_speed(0.0f), microstep(MICROSTEP_1)
{
    pwm.set_duty_percent(50.0f);
    apply_microstep();
}

void Motor::set_speed(float rad_s) {
    if (rad_s < 0.0f) rad_s = 0.0f;
    current_speed = rad_s;
    float freq = rad_s * (float)STEPS_PER_REV / (2.0f * MOTOR_PI);
    if (freq < 1.0f) freq = 0.0f;
    pwm.set_freq((uint32_t)freq);
}

float Motor::get_position_rad() {
    uint32_t raw = adc.read_channel(adc_channel);
    return (float)raw * 2.0f * MOTOR_PI / 4095.0f;
}

void Motor::set_microstep(uint8_t step) {
    switch (step) {
        case MICROSTEP_1:  microstep = 0; break;
        case MICROSTEP_2:  microstep = 1; break;
        case MICROSTEP_4:  microstep = 2; break;
        case MICROSTEP_8:  microstep = 3; break;
        case MICROSTEP_16: microstep = 4; break;
        default:           microstep = 0; break;
    }
    apply_microstep();
}

float Motor::get_speed() const {
    return current_speed;
}

void Motor::apply_microstep() {
    ms1.write(microstep_table[microstep][0]);
    ms2.write(microstep_table[microstep][1]);
    ms3.write(microstep_table[microstep][2]);
}
