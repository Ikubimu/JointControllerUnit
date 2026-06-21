#include "Control.hpp"

Control *Control::s_instance = nullptr;

Control& Control::getInstance(Motor &motor) {
    if (s_instance == nullptr)
        s_instance = new Control(motor);
    return *s_instance;
}

Control& Control::getInstance() {
    return *s_instance;
}

Control::Control(Motor &motor) : motor(motor) {}

void Control::Move(float position, float speed) {
    (void)position;
    motor.setMovement(speed);
}

void Control::Stop() {
    motor.stop();
}
