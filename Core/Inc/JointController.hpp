#ifndef JOINT_CONTROLLER_HPP
#define JOINT_CONTROLLER_HPP

#include <stdio.h>
#include "DigitalOutput.hpp"
#include "PWM.hpp"
#include "ADC.hpp"
#include "CAN.hpp"
#include "StateMachine/StateMachine.hpp"
#include "FreeRTOS.h"
#include "task.h"

void vTaskCAN(void *pvParameters);
void vTaskADC(void *pvParameters);
void vTaskLED(void *pvParameters);
void vTaskLEDPB0(void *pvParameters);
void jointMainTask(void *pvParameters);

#endif
