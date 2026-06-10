#include "CommunicationHandler.hpp"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

CommunicationHandler::CommunicationHandler(CAN_HandleTypeDef *hcan)
    : can(hcan) {
}

bool CommunicationHandler::start() {
    if (!can.start(0, 0))
        return false;

    if (xTaskCreate(taskFunction, "CAN", 128, this, 1, NULL) != pdPASS)
        return false;

    return true;
}

bool CommunicationHandler::setFilter(uint8_t device_id) {
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

    return HAL_CAN_ConfigFilter(can.getHandle(), &canFilter) == HAL_OK;
}

void CommunicationHandler::taskFunction(void *pvParameters) {
    CommunicationHandler *self = static_cast<CommunicationHandler*>(pvParameters);
    self->run();
}

void CommunicationHandler::run() {
    for (;;) {
        while (HAL_CAN_GetRxFifoFillLevel(can.getHandle(), CAN_RX_FIFO0) > 0) {
            CAN_Message rxMsg;
            if (can.read_message(&rxMsg)) {
                printf("CAN RX: ID=0x%03lX %s DLC=%u Data=",
                       rxMsg.id, rxMsg.is_extended ? "EXT" : "STD", rxMsg.dlc);
                for (uint8_t i = 0; i < rxMsg.dlc; i++)
                    printf("%02X ", rxMsg.data[i]);
                printf("\r\n");

                if (can.write_message(&rxMsg))
                    printf("CAN TX echo OK\r\n");
                else
                    printf("CAN TX echo ERROR\r\n");
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
