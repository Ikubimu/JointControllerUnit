#include "Peripherals.hpp"

extern CAN_HandleTypeDef hcan;
extern ADC_HandleTypeDef hadc1;

DigitalOutput led(GPIOC, GPIO_PIN_13);
DigitalOutput ledPB0(GPIOB, GPIO_PIN_0);
DigitalOutput out1(GPIOB, GPIO_PIN_1);
DigitalOutput idlePIN(GPIOB, GPIO_PIN_10);
DigitalOutput faultPIN(GPIOB, GPIO_PIN_11);
ADC adc(&hadc1);
CAN can(&hcan);
CAN_Message canMsg;
