#include "Motor.hpp"
#include "FreeRTOS.h"
#include <stdio.h>
#include "task.h"

static const uint8_t microstep_table[5][3] = {
    {0, 0, 0},  // MICROSTEP_1  (full)
    {1, 0, 0},  // MICROSTEP_2  (half)
    {0, 1, 0},  // MICROSTEP_4  (quarter)
    {1, 1, 0},  // MICROSTEP_8  (eighth)
    {1, 1, 1},  // MICROSTEP_16 (sixteenth)
};

static volatile float   s_position_deg = 0.0f;
static volatile float   s_actual_speed = 0.0f;
static volatile float   s_angle_deg = 0.0f;
static volatile bool    s_direction = DIR_CW;

static int32_t          s_rot_count = 0;
static float            s_ratio = GEAR_RATIO;

Motor *Motor::s_instance = nullptr;

static void vTaskEncoder(void *pvParameters) {
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    static float prev_pos = 0.0f;
    static bool first_run = true;

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(ENCODER_READ_MS));

        uint32_t curr_raw = Motor::getInstance().get_position_raw();
        s_angle_deg = (float)curr_raw * 360.0f / 4095.0f;

        if (first_run) {
            prev_pos = s_angle_deg;
            first_run = false;
            continue;
        }

        float delta = s_angle_deg - prev_pos;

        if(abs(delta) > 180.0F && prev_pos > 180.0F) {
            s_rot_count++;
            delta = delta + 360.0F;
        } else if(abs(delta) > 180.0F && prev_pos < 180.0F) {
            s_rot_count--;
            delta = delta - 360.0F;
        }
        s_position_deg = s_angle_deg / s_ratio + (360.0f / s_ratio) * (float)s_rot_count;
        s_actual_speed = delta / ((float)ENCODER_READ_MS * 0.001f * s_ratio);
        prev_pos = s_angle_deg;

        Motor::getInstance().kalman.update(s_angle_deg, s_actual_speed, s_actual_speed);
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
    return kalman.get_velocity();
}

bool Motor::get_direction() {
    return s_direction;
}

Motor::Motor(PWM &pwm, ADC &adc, uint32_t adc_channel,
             DigitalOutput &ms1, DigitalOutput &ms2, DigitalOutput &ms3,
             DigitalOutput &dir, float ratio)
    : pwm(pwm), adc(adc), adc_channel(adc_channel),
      ms1(ms1), ms2(ms2), ms3(ms3), dir(dir),
      microstep(0), kalman(0.0f, 0.0f)
{
    s_ratio = ratio;
    set_direction(DIR_CW);
    calibration(0.0f, ratio);
    set_microstep(MICROSTEP_1);
    xTaskCreate(vTaskEncoder, "Encoder", 128, NULL, 1, NULL);
}

void Motor::set_speed(float deg_s) {
    if (deg_s < 0.0f) deg_s = 0.0f;
    float freq = deg_s * (float)STEPS_PER_REV / 360.0f;
    if (freq < 1.0f) freq = 0.0f;
    pwm.set_freq((uint32_t)freq*8);
}

void Motor::set_direction(bool cw) {
    s_direction = cw;
    dir.write(cw);
}

void Motor::set_ratio(float ratio) {
    s_ratio = ratio;
}

float Motor::get_position_deg() {
    return s_position_deg;
}

float Motor::get_angle_deg() {
    return kalman.get_position();
}

void Motor::set_position_deg(float deg) {
    s_position_deg = deg;
}

void Motor::setMovement(float deg_s) {
    deg_s = deg_s*s_ratio;
    if(deg_s == 0.0f) {
        stop();
        return;
    }
    if (deg_s < 0.0f) {
        set_direction(DIR_CCW);
        set_speed(-deg_s);
    } else {
        set_direction(DIR_CW);
        set_speed(deg_s);
    }
    if(!motor_on)
    {
        pwm.set_duty_percent(50.0f);
        pwm.start();
        motor_on = true;
    }
}

void Motor::calibration(float pos_deg, float ratio) {
    set_position_deg(pos_deg);
    set_ratio(ratio);
    kalman.init(pos_deg, 0.0f);
}

void Motor::stop() {
    pwm.stop();
    motor_on = false;
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
