#ifndef CONTROL_HPP
#define CONTROL_HPP

#include "Motor.hpp"

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

    Motor &motor;
    static Control *s_instance;
};

#endif
