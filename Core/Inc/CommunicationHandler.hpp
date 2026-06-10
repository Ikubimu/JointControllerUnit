#ifndef COMMUNICATION_HANDLER_HPP
#define COMMUNICATION_HANDLER_HPP

#include <stdint.h>
#include <functional>
#include "CAN.hpp"

class CommunicationHandler {
public:
    CommunicationHandler() = delete;

    static bool start(uint8_t device_id);
    static bool registerService(uint16_t canId,
                                std::function<void(const CAN_Message*)> callback);
private:
    static constexpr uint8_t MAX_SERVICES = 8;

    struct ServiceEntry {
        uint16_t id;
        std::function<void(const CAN_Message*)> callback;
    };

    static void taskFunction(void *pvParameters);
    static void run();

    static ServiceEntry services[MAX_SERVICES];
    static uint8_t numServices;
};

#endif
