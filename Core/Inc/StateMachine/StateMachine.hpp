#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

#include <stdint.h>
#include <functional>
#include "State.hpp"

enum StateID : uint8_t {
    INIT = 0,
    IDLE,
    FAULT,
    STAN_BY,
    ACTION,
    PAUSE
};

class StateMachine {
public:
    static StateMachine& get();

    bool addState(uint8_t id, uint8_t index);
    State* getState(uint8_t index);
    uint8_t getNumStates() const;

    bool addTransition(uint8_t stateIndex, std::function<bool()> condition, uint8_t targetId);
    bool addEntryAction(uint8_t stateIndex, std::function<void()> action);
    bool addExitAction(uint8_t stateIndex, std::function<void()> action);
    void setCurrentState(uint8_t index);
    uint8_t getCurrentState() const;
    void update();

private:
    StateMachine();
    StateMachine(const StateMachine&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;

    uint8_t findIndexById(uint8_t id) const;

    State states[6];
    uint8_t numStates;
    uint8_t currentState;
};

#endif
