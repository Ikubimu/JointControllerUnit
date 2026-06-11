#ifndef LOW_PERIPHERALS_HPP
#define LOW_PERIPHERALS_HPP

#include "DigitalOutput.hpp"
#include "PWM.hpp"
#include "ADC.hpp"

extern DigitalOutput led;
extern DigitalOutput ledPB0;
extern DigitalOutput out1;
extern DigitalOutput idlePIN;
extern DigitalOutput faultPIN;
extern ADC adc;
extern PWM pwm;
extern DigitalOutput ms1;
extern DigitalOutput ms2;
extern DigitalOutput ms3;
extern DigitalOutput dir;

#endif
