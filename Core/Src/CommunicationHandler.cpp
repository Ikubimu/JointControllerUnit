#include "CommunicationHandler.hpp"
#include "CAN.hpp"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

extern CAN_HandleTypeDef hcan;
static CAN canInstance(&hcan);

bool CommunicationHandler::start(uint8_t device_id) {
    static bool initialized = false;
    if (initialized) return true;
    initialized = true;

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

void CommunicationHandler::taskFunction(void *pvParameters) {
    (void)pvParameters;
    run();
}

void CommunicationHandler::run() {
    for (;;) {
        while (HAL_CAN_GetRxFifoFillLevel(canInstance.getHandle(), CAN_RX_FIFO0) > 0) {
            CAN_Message rxMsg;
            if (canInstance.read_message(&rxMsg)) {
                printf("CAN RX: ID=0x%03lX %s DLC=%u Data=",
                       rxMsg.id, rxMsg.is_extended ? "EXT" : "STD", rxMsg.dlc);
                for (uint8_t i = 0; i < rxMsg.dlc; i++)
                    printf("%02X ", rxMsg.data[i]);
                printf("\r\n");

                if (canInstance.write_message(&rxMsg))
                    printf("CAN TX echo OK\r\n");
                else
                    printf("CAN TX echo ERROR\r\n");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
