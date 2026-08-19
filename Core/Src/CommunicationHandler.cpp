#include "CommunicationHandler.hpp"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

extern CAN_HandleTypeDef hcan;
static CAN canInstance(&hcan);

CommunicationHandler::ServiceEntry CommunicationHandler::services[MAX_SERVICES] = {};
uint8_t CommunicationHandler::numServices = 0;
uint8_t CommunicationHandler::deviceId = 0;

QueueHandle_t CommunicationHandler::txQueue = nullptr;

bool CommunicationHandler::start(uint8_t device_id) {
    static bool initialized = false;
    if (initialized) return true;
    initialized = true;

    deviceId = device_id;
    txQueue = xQueueCreate(TX_QUEUE_SIZE, sizeof(CAN_Message));

    if (!canInstance.start(0, 0))
        return false;

    CAN_FilterTypeDef canFilter = {0};
    canFilter.FilterBank = 0;
    canFilter.FilterMode = CAN_FILTERMODE_IDMASK;
    canFilter.FilterScale = CAN_FILTERSCALE_16BIT;
    canFilter.FilterIdHigh = (device_id << 13);
    canFilter.FilterIdLow = 0;
    canFilter.FilterMaskIdHigh = 0xE018;
    canFilter.FilterMaskIdLow = 0xE018;
    canFilter.FilterFIFOAssignment = CAN_RX_FIFO0;
    canFilter.FilterActivation = CAN_FILTER_ENABLE;
    HAL_CAN_ConfigFilter(canInstance.getHandle(), &canFilter);

    return xTaskCreate(taskFunction, "CAN", 128, NULL, 1, NULL) == pdPASS;
}

bool CommunicationHandler::registerService(uint8_t canCmd,
        std::function<void(const CAN_Message*)> callback) {
    if (numServices >= MAX_SERVICES)
        return false;
    uint16_t canId = ((uint16_t)deviceId << 8) | canCmd;
    services[numServices] = {canId, std::move(callback)};
    numServices++;
    return true;
}

bool CommunicationHandler::registerMasterService(uint8_t canCmd,
        std::function<void(const CAN_Message*)> callback) {
    if (numServices >= MAX_SERVICES)
        return false;
    uint16_t canId = (uint16_t)canCmd;
    services[numServices] = {canId, std::move(callback)};
    numServices++;
    return true;
}

bool CommunicationHandler::updateJointStatus(float val1, float val2) {
    CAN_Message msg;
    msg.id = ((uint32_t)deviceId << 8) | CMD_STATUS;
    msg.dlc = 8;
    msg.is_extended = false;
    memcpy(&msg.data[0], &val1, sizeof(float));
    memcpy(&msg.data[4], &val2, sizeof(float));
    return xQueueSend(txQueue, &msg, 0) == pdTRUE;
}

void CommunicationHandler::taskFunction(void *pvParameters) {
    (void)pvParameters;
    run();
}

void CommunicationHandler::processTxQueue() {
    CAN_Message msg;
    while (xQueuePeek(txQueue, &msg, 0) == pdTRUE) {
        if (canInstance.write_message(&msg)) {
            xQueueReceive(txQueue, &msg, 0);
        } else {
            break;
        }
    }
}

void CommunicationHandler::run() {
    for (;;) {
        while (HAL_CAN_GetRxFifoFillLevel(canInstance.getHandle(), CAN_RX_FIFO0) > 0) {
            CAN_Message rxMsg;
            if (canInstance.read_message(&rxMsg)) {
                bool handled = false;
                for (uint8_t i = 0; i < numServices; i++) {
                    if (services[i].id == rxMsg.id) {
                        services[i].callback(&rxMsg);
                        handled = true;
                        break;
                    }
                }
                if (!handled) {
                    printf("CAN RX (unhandled): ID=0x%03lX DLC=%u Data=",
                           rxMsg.id, rxMsg.dlc);
                    for (uint8_t i = 0; i < rxMsg.dlc; i++)
                        printf("%02X ", rxMsg.data[i]);
                    printf("\r\n");
                }

                if (canInstance.write_message(&rxMsg))
                    printf("CAN TX echo OK\r\n");
                else
                    printf("CAN TX echo ERROR\r\n");
            }
        }

        processTxQueue();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
