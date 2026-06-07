#include "Motor.hpp"
#include "FreeRTOS.h"
#include "task.h"

#define ENCODER_READ_MS 100

static const uint8_t microstep_table[5][3] = {
    {0, 0, 0},  // MICROSTEP_1  (full)
    {1, 0, 0},  // MICROSTEP_2  (half)
    {0, 1, 0},  // MICROSTEP_4  (quarter)
    {1, 1, 0},  // MICROSTEP_8  (eighth)
    {1, 1, 1},  // MICROSTEP_16 (sixteenth)
};

static volatile float   s_position_rad = 0.0f;
static volatile float   s_actual_speed = 0.0f;
static volatile bool    s_direction = DIR_CW;

static int32_t          s_rot_count = 0;
static float            s_ratio = GEAR_RATIO;

Motor *Motor::s_instance = nullptr;

static void vTaskEncoder(void *pvParameters) {
    (void)pvParameters;
    static const float delta_sec = (float)ENCODER_READ_MS * 0.001f;
    uint32_t prev_raw = Motor::getInstance().get_position_raw();
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(ENCODER_READ_MS));
        uint32_t curr_raw = Motor::getInstance().get_position_raw();
        int32_t delta_raw = (int32_t)curr_raw - (int32_t)prev_raw;

        if (s_direction == DIR_CW) {
            if (delta_raw < 0) {
                delta_raw += 4096;
                s_rot_count++;
            }
        } else {
            if (delta_raw > 0) {
                delta_raw -= 4096;
                s_rot_count--;
            }
        }

        float frac = (float)curr_raw * 2.0f * MOTOR_PI / 4095.0f;
        s_position_rad = frac / s_ratio + (2.0f * MOTOR_PI / s_ratio) * (float)s_rot_count;
        s_actual_speed = (float)delta_raw * 2.0f * MOTOR_PI / (4095.0f * delta_sec * s_ratio);
        prev_raw = curr_raw;
    }
}

Motor& Motor::getInstance(PWM &pwm, ADC &adc, uint32_t adc_channel,
                          DigitalOutput &ms1, DigitalOutput &ms2, DigitalOutput &ms3,
                          DigitalOutput &dir, float ratio) {
    if (s_instance == nullptr)
        s_instance = new Motor(pwm, adc, adc_channel, ms1, ms2, ms3, dir, ratio);
    return *s_instance;
}

Motor& Motor::getInstance() {
    return *s_instance;
}

float Motor::get_actual_speed() {
    return s_actual_speed;
}

bool Motor::get_direction() {
    return s_direction;
}

Motor::Motor(PWM &pwm, ADC &adc, uint32_t adc_channel,
             DigitalOutput &ms1, DigitalOutput &ms2, DigitalOutput &ms3,
             DigitalOutput &dir, float ratio)
    : pwm(pwm), adc(adc), adc_channel(adc_channel),
      ms1(ms1), ms2(ms2), ms3(ms3), dir(dir),
      microstep(MICROSTEP_1)
{
    s_ratio = ratio;
    pwm.set_duty_percent(50.0f);
    set_direction(DIR_CW);
    apply_microstep();
    xTaskCreate(vTaskEncoder, "Encoder", 128, NULL, 1, NULL);
}

void Motor::set_speed(float rad_s) {
    if (rad_s < 0.0f) rad_s = 0.0f;
    float freq = rad_s * (float)STEPS_PER_REV / (2.0f * MOTOR_PI);
    if (freq < 1.0f) freq = 0.0f;
    pwm.set_freq((uint32_t)freq);
}

void Motor::set_direction(bool cw) {
    s_direction = cw;
    dir.write(cw);
}

void Motor::set_ratio(float ratio) {
    s_ratio = ratio;
}

float Motor::get_position_rad() {
    return s_position_rad;
}

uint32_t Motor::get_position_raw() {
    return adc.read_channel(adc_channel);
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

void Motor::apply_microstep() {
    ms1.write(microstep_table[microstep][0]);
    ms2.write(microstep_table[microstep][1]);
    ms3.write(microstep_table[microstep][2]);
}
