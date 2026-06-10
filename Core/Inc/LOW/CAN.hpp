#ifndef LOW_CAN_HPP
#define LOW_CAN_HPP

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t id;
    uint8_t data[8];
    uint8_t dlc;
    bool is_extended;
} CAN_Message;

class CAN {
public:
    CAN(CAN_HandleTypeDef *hcan);
    bool start(uint32_t filter_id, uint32_t filter_mask);
    bool write_message(const CAN_Message *msg);
    bool read_message(CAN_Message *msg);
    CAN_HandleTypeDef* getHandle();
private:
    CAN_HandleTypeDef *hcan;
    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;
};

#endif
