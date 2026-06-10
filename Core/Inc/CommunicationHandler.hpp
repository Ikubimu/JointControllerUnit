#ifndef COMMUNICATION_HANDLER_HPP
#define COMMUNICATION_HANDLER_HPP

#include <stdint.h>

class CommunicationHandler {
public:
    CommunicationHandler() = delete;

    static bool start(uint8_t device_id);
private:
    static void taskFunction(void *pvParameters);
    static void run();
};

#endif
