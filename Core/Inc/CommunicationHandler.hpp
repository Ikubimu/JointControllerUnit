#ifndef COMMUNICATION_HANDLER_HPP
#define COMMUNICATION_HANDLER_HPP

#include "stm32f1xx_hal.h"
#include "CAN.hpp"

class CommunicationHandler {
public:
    CommunicationHandler(CAN_HandleTypeDef *hcan);
    bool start();
    bool setFilter(uint8_t device_id);
private:
    CAN can;

    static void taskFunction(void *pvParameters);
    void run();
};

#endif
