#include "Control.hpp"
#include <math.h>

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
      targetPos(0), targetSpeed(0) {
    xTaskCreate(taskFunction, "Control", 128, this, 1, &taskHandle);
}

void Control::taskFunction(void *pvParameters) {
    Control *self = static_cast<Control*>(pvParameters);
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        self->running = true;
        while (self->running) {
            float error = self->targetPos - self->motor.get_position_deg();
            if (fabs(error) < DEADBAND) {
                self->motor.stop();
                break;
            }
            if (error > 0)
                self->motor.setMovement(self->targetSpeed);
            else
                self->motor.setMovement(-self->targetSpeed);
            vTaskDelay(pdMS_TO_TICKS(2 * ENCODER_READ_MS));
        }
    }
}

void Control::Move(float position, float speed) {
    targetPos = position;
    targetSpeed = speed;
    xTaskNotifyGive(taskHandle);
}

void Control::Stop() {
    running = false;
}
