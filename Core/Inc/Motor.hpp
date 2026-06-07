#ifndef LOW_MOTOR_HPP
#define LOW_MOTOR_HPP

#include "PWM.hpp"
#include "ADC.hpp"
#include "DigitalOutput.hpp"
#include <math.h>

#define MOTOR_PI 3.14159265358979323846f

#define STEPS_PER_REV  200

#define MICROSTEP_1     1
#define MICROSTEP_2     2
#define MICROSTEP_4     4
#define MICROSTEP_8     8
#define MICROSTEP_16   16

class Motor {
public:
    Motor(PWM &pwm, ADC &adc, uint32_t adc_channel,
          DigitalOutput &ms1, DigitalOutput &ms2, DigitalOutput &ms3);

    void    set_speed(float rad_s);
    float   get_position_rad();
    void    set_microstep(uint8_t step);
    float   get_speed() const;

private:
    PWM          &pwm;
    ADC          &adc;
    uint32_t     adc_channel;
    DigitalOutput &ms1;
    DigitalOutput &ms2;
    DigitalOutput &ms3;

    float        current_speed;
    uint8_t      microstep;

    void apply_microstep();
};

#endif
