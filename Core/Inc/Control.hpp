#ifndef CONTROL_HPP
#define CONTROL_HPP

#include "Motor.hpp"
#include "FreeRTOS.h"
#include "task.h"

class Control {
public:
    static Control& getInstance(Motor &motor);
    static Control& getInstance();

    void Move(float position, float speed);
    void Stop();

private:
    Control(Motor &motor);
    ~Control() = default;
    Control(const Control&) = delete;
    Control& operator=(const Control&) = delete;

    static void taskFunction(void *pvParameters);

    Motor &motor;
    TaskHandle_t taskHandle;
    volatile bool running;
    float targetPos;
    float targetSpeed;
    static Control *s_instance;
};

#endif
