#ifndef CONTROL_HPP
#define CONTROL_HPP

#include "Motor.hpp"
#include "FreeRTOS.h"
#include "task.h"

#define CONTROL_PERIOD_MS      20
#define SIGMOID_K              0.25f
#define SIGMOID_X0             20.0f
#define BRAKING_DISTANCE_DEG   30.0f

class Control {
public:
    static Control& getInstance(Motor &motor);
    static Control& getInstance();

    void Move(float position, float speed);
    void SetSpeed(float deg_s);
    void Stop();

    float GetTarget();
    float GetOutput();

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

    float target;
    float output;
    float startOut;
    float braking_distance;
    int   startI;
    bool  braking;

    static Control *s_instance;
};

#endif
