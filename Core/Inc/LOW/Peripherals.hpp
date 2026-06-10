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

#endif
