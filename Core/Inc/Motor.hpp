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

#define DIR_CW  0
#define DIR_CCW 1

class Motor {
public:
    static Motor& getInstance(PWM &pwm, ADC &adc, uint32_t adc_channel,
                              DigitalOutput &ms1, DigitalOutput &ms2, DigitalOutput &ms3,
                              DigitalOutput &dir);
    static Motor& getInstance();
    static uint32_t get_raw_encoder();
    static float   get_actual_speed();
    static bool    get_direction();

    void    set_speed(float rad_s);
    void    set_direction(bool cw);
    float   get_position_rad();
    uint32_t get_position_raw();
    void    set_microstep(uint8_t step);

private:
    Motor(PWM &pwm, ADC &adc, uint32_t adc_channel,
          DigitalOutput &ms1, DigitalOutput &ms2, DigitalOutput &ms3,
          DigitalOutput &dir);
    ~Motor() = default;
    Motor(const Motor&) = delete;
    Motor& operator=(const Motor&) = delete;

    PWM          &pwm;
    ADC          &adc;
    uint32_t     adc_channel;
    DigitalOutput &ms1;
    DigitalOutput &ms2;
    DigitalOutput &ms3;
    DigitalOutput &dir;

    uint8_t      microstep;

    void apply_microstep();

    static Motor *s_instance;
};

#endif
