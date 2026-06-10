#ifndef JOINT_CONTROLLER_HPP
#define JOINT_CONTROLLER_HPP

#include <stdio.h>

void vTaskADC(void *pvParameters);
void vTaskLED(void *pvParameters);
void vTaskLEDPB0(void *pvParameters);
void jointMainTask(void *pvParameters);

#endif
