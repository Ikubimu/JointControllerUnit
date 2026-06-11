#include "Peripherals.hpp"

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;

DigitalOutput led(GPIOC, GPIO_PIN_13);
DigitalOutput ledPB0(GPIOB, GPIO_PIN_0);
DigitalOutput out1(GPIOB, GPIO_PIN_1);
DigitalOutput idlePIN(GPIOB, GPIO_PIN_10);
DigitalOutput faultPIN(GPIOB, GPIO_PIN_11);
ADC adc(&hadc1);
PWM pwm(&htim1, TIM_CHANNEL_1, 8000000);
DigitalOutput ms1(GPIOB, GPIO_PIN_12);
DigitalOutput ms2(GPIOB, GPIO_PIN_13);
DigitalOutput ms3(GPIOB, GPIO_PIN_14);
DigitalOutput dir(GPIOB, GPIO_PIN_15);
