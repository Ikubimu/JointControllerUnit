#include "Control.hpp"
#include "Protections.hpp"
#include <math.h>
#include <stdio.h>

static constexpr float DEADBAND = 0.5f;

Control *Control::s_instance = nullptr;

Control& Control::getInstance(Motor &motor) {
    if (s_instance == nullptr)
        s_instance = new Control(motor);
    return *s_instance;
}

Control& Control::getInstance() {
    return *s_instance;
}

Control::Control(Motor &motor)
    : motor(motor), taskHandle(nullptr), running(false),
      targetPos(0), targetSpeed(0),
      target(0), output(0), startOut(0), startI(0), braking(false) {
    xTaskCreate(taskFunction, "Control", 256, this, 2, &taskHandle);
}

void Control::taskFunction(void *pvParameters) {
    Control *self = static_cast<Control*>(pvParameters);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        self->running = true;
        self->braking = false;
        self->startI = 0;
        self->startOut = self->output;
        Protections::resetMovement();

        while (self->running) {
            vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(CONTROL_PERIOD_MS));

            float error = self->targetPos - self->motor.get_position_deg();

            if (fabs(error) < DEADBAND) {
                self->motor.stop();
                self->running = false;
                break;
            }

            if (!self->braking && fabs(error) < self->braking_distance) {
                self->braking = true;
                self->startOut = self->output;
                self->target = 0.0f;
                self->startI = 0;
            }

            if (Protections::checkMovement(error)) {
                self->motor.stop();
                self->running = false;
                break;
            }

            float L = self->target - self->startOut;
            self->output = self->startOut + L / (1.0f + expf(-SIGMOID_K * ((float)self->startI - SIGMOID_X0)));
            self->startI++;

            self->motor.setMovement(self->output);
        }
    }
}

void Control::Move(float position, float speed) {
    targetPos = position;
    targetSpeed = speed;
    target = speed;
    braking = false;
    Protections::resetMovement();
    braking_distance = abs((4.0f * speed)/SIGMOID_K * (CONTROL_PERIOD_MS * 0.001f));
    xTaskNotifyGive(taskHandle);
}

void Control::SetSpeed(float deg_s) {
    startOut = output;
    target = deg_s;
    startI = 0;
    braking = false;
}

void Control::Stop() {
    running = false;
}

float Control::GetTarget() { return target; }
float Control::GetOutput() { return output; }
