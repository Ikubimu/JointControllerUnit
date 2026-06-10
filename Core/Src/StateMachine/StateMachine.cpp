#include "StateMachine.hpp"

StateMachine& StateMachine::get() {
    static StateMachine root;
    static StateMachine sub;
    static bool init = false;

    if (!init) {
        init = true;

        root.addState(INIT, 0);
        root.addState(IDLE, 1);
        root.addState(FAULT, 2);

        sub.addState(STAN_BY, 0);
        sub.addState(ACTION, 1);
        sub.addState(PAUSE, 2);
        root.states[IDLE].setSubMachine(&sub);
    }

    return root;
}

StateMachine::StateMachine()
    : numStates(0), currentState(0) {
}

bool StateMachine::addState(uint8_t id, uint8_t index) {
    if (index >= 6)
        return false;
    states[index] = State(id);
    if (index >= numStates)
        numStates = index + 1;
    return true;
}

State* StateMachine::getState(uint8_t index) {
    if (index >= 6)
        return nullptr;
    return &states[index];
}

uint8_t StateMachine::getNumStates() const {
    return numStates;
}

bool StateMachine::addTransition(uint8_t stateIndex, std::function<bool()> condition, uint8_t targetId) {
    if (stateIndex >= 6)
        return false;
    return states[stateIndex].addTransition(condition, targetId);
}

bool StateMachine::addEntryAction(uint8_t stateIndex, std::function<void()> action) {
    if (stateIndex >= 6)
        return false;
    return states[stateIndex].addEntryAction(action);
}

bool StateMachine::addExitAction(uint8_t stateIndex, std::function<void()> action) {
    if (stateIndex >= 6)
        return false;
    return states[stateIndex].addExitAction(action);
}

void StateMachine::setCurrentState(uint8_t index) {
    if (index < 6)
        currentState = index;
}

uint8_t StateMachine::getCurrentState() const {
    return currentState;
}

void StateMachine::update() {
    uint8_t targetId = states[currentState].checkTransitions();
    if (targetId != 0xFF) {
        uint8_t idx = findIndexById(targetId);
        if (idx < 6) {
            states[currentState].runExitActions();
            currentState = idx;
            states[currentState].runEntryActions();
        }
    }
    states[currentState].updateSubMachine();
}

uint8_t StateMachine::findIndexById(uint8_t id) const {
    for (uint8_t i = 0; i < numStates; i++) {
        if (states[i].getId() == id)
            return i;
    }
    return 0xFF;
}
