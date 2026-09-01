#ifndef COMMUNICATION_HANDLER_HPP
#define COMMUNICATION_HANDLER_HPP

#include <stdint.h>
#include <functional>
#include "CAN.hpp"
#include "FlagUtils.hpp"
#include "FreeRTOS.h"
#include "queue.h"

// COMMANDS DEFINITIONS

#define CMD_ERROR        0x01
#define CMD_START        0x02
#define CMD_CALIBRATION  0x03
#define CMD_ANNOUNCE     0x04
#define CMD_MOVE_TARGET  0x05
#define CMD_PAUSE        0x06
#define CMD_RESUME       0x07
#define CMD_STATUS       0x08
#define CMD_WATCHDOG     0X09

#define PROPAGATE_ERROR_LIMIT 3

// Error codes definitions
#define NO_ERROR                        0x00
#define ENCODER_ERROR                   0x01
#define COLISION_ERROR                  0x02
#define OUT_OF_RANGE_ERROR              0x03
#define MOVE_FAILURE_ERROR              0x04
#define CALIBRATION_FAILURE_ERROR       0x05
#define COMMUNICATION_LOST_ERROR        0x06
#define UNKNOWN_ERROR                   0xAA


class CommunicationHandler {
public:
    CommunicationHandler() = delete;

    static bool start(uint8_t device_id);
    static bool registerService(uint8_t canCmd,
                                std::function<void(const CAN_Message*)> callback);
    static bool registerMasterService(uint8_t canCmd,
                                      std::function<void(const CAN_Message*)> callback);
    static bool updateJointStatus(float val1, float val2);
    static bool announceDevice(uint8_t device_id);
    static bool propagateError(uint8_t errorCode);
private:
    static constexpr uint8_t MAX_SERVICES = 8;
    static constexpr uint8_t TX_QUEUE_SIZE = 10;

    struct ServiceEntry {
        uint16_t id;
        std::function<void(const CAN_Message*)> callback;
    };

    static void taskFunction(void *pvParameters);
    static void run();
    static void processTxQueue();

    static ServiceEntry services[MAX_SERVICES];
    static uint8_t numServices;
    static uint8_t deviceId;

    static QueueHandle_t txQueue;
};

#endif
