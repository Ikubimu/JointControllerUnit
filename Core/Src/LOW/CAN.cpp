#include "CAN.hpp"

CAN::CAN(CAN_HandleTypeDef *hcan)
    : hcan(hcan), txMailbox(0) {}

bool CAN::start(uint32_t filter_id, uint32_t filter_mask) {
    CAN_FilterTypeDef canFilter = {0};
    canFilter.FilterBank = 0;
    canFilter.FilterMode = CAN_FILTERMODE_IDMASK;
    canFilter.FilterScale = CAN_FILTERSCALE_32BIT;
    canFilter.FilterIdHigh = (uint16_t)(filter_id << 5);
    canFilter.FilterIdLow = 0;
    canFilter.FilterMaskIdHigh = (uint16_t)(filter_mask << 5);
    canFilter.FilterMaskIdLow = 0;
    canFilter.FilterFIFOAssignment = CAN_RX_FIFO0;
    canFilter.FilterActivation = CAN_FILTER_ENABLE;

    if (HAL_CAN_ConfigFilter(hcan, &canFilter) != HAL_OK)
        return false;

    txHeader.StdId = 0;
    txHeader.ExtId = 0;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.DLC = 8;
    txHeader.TransmitGlobalTime = DISABLE;

    return HAL_CAN_Start(hcan) == HAL_OK;
}

bool CAN::write_message(const CAN_Message *msg) {
    if (msg->is_extended) {
        txHeader.IDE = CAN_ID_EXT;
        txHeader.StdId = 0;
        txHeader.ExtId = msg->id;
    } else {
        txHeader.IDE = CAN_ID_STD;
        txHeader.StdId = msg->id;
        txHeader.ExtId = 0;
    }
    txHeader.DLC = msg->dlc;
    txHeader.RTR = CAN_RTR_DATA;

    return HAL_CAN_AddTxMessage(hcan, &txHeader,
                                (uint8_t *)msg->data, &txMailbox) == HAL_OK;
}

bool CAN::read_message(CAN_Message *msg) {
    CAN_RxHeaderTypeDef rxHeader = {0};
    uint8_t rxData[8] = {0};

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) != HAL_OK)
        return false;

    if (rxHeader.IDE == CAN_ID_EXT) {
        msg->id = rxHeader.ExtId;
        msg->is_extended = true;
    } else {
        msg->id = rxHeader.StdId;
        msg->is_extended = false;
    }
    msg->dlc = rxHeader.DLC;
    for (uint8_t i = 0; i < msg->dlc && i < 8; i++)
        msg->data[i] = rxData[i];

    return true;
}
