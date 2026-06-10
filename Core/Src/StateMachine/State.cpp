#include "State.hpp"
#include "StateMachine.hpp"

bool State::addTransition(std::function<bool()> condition, uint8_t targetId) {
    if (numTransitions >= MAX_TRANSITIONS)
        return false;
    transitions[numTransitions].condition = condition;
    transitions[numTransitions].targetId = targetId;
    numTransitions++;
    return true;
}

uint8_t State::checkTransitions() const {
    for (uint8_t i = 0; i < numTransitions; i++) {
        if (transitions[i].condition())
            return transitions[i].targetId;
    }
    return 0xFF;
}

bool State::addEntryAction(std::function<void()> action) {
    if (numEntryActions >= MAX_ACTIONS)
        return false;
    entryActions[numEntryActions] = action;
    numEntryActions++;
    return true;
}

void State::runEntryActions() {
    for (uint8_t i = 0; i < numEntryActions; i++)
        entryActions[i]();
    if (subMachine != nullptr) {
        subMachine->setCurrentState(0);
        subMachine->getState(0)->runEntryActions();
    }
}

bool State::addExitAction(std::function<void()> action) {
    if (numExitActions >= MAX_ACTIONS)
        return false;
    exitActions[numExitActions] = action;
    numExitActions++;
    return true;
}

void State::runExitActions() {
    if (subMachine != nullptr)
        subMachine->getState(subMachine->getCurrentState())->runExitActions();
    for (uint8_t i = 0; i < numExitActions; i++)
        exitActions[i]();
}

void State::updateSubMachine() {
    if (subMachine != nullptr)
        subMachine->update();
}
