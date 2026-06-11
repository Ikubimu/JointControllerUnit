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

#define GEAR_RATIO 22.0f

class Motor {
public:
    static Motor& getInstance(PWM &pwm, ADC &adc, uint32_t adc_channel,
                              DigitalOutput &ms1, DigitalOutput &ms2, DigitalOutput &ms3,
                              DigitalOutput &dir, float ratio = GEAR_RATIO);
    static Motor& getInstance();
    static float   get_actual_speed();
    static bool    get_direction();

    float   get_position_rad();
    uint32_t get_position_raw();
    void    set_microstep(uint8_t step);
   
    void    setMovement(float rad_s);
    void    stop();
    void    calibration(float pos_rad, float ratio);

private:
    Motor(PWM &pwm, ADC &adc, uint32_t adc_channel,
          DigitalOutput &ms1, DigitalOutput &ms2, DigitalOutput &ms3,
          DigitalOutput &dir, float ratio);
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
    bool         motor_on = false;

    void apply_microstep();
    void set_position_rad(float rad);
    void set_ratio(float ratio);
    void set_direction(bool cw);
    void set_speed(float rad_s);

    static Motor *s_instance;
};

#endif
