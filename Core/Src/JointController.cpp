#include "JointController.hpp"
#include "CommunicationHandler.hpp"
#include "Motor.hpp"
#include "Control.hpp"
#include "Protections.hpp"
#include "Peripherals.hpp"
#include "StateMachine/StateMachine.hpp"
#include "FlagUtils.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

uint8_t deviceId = 0;

static uint32_t lastMasterMsg = 0;

void publishJointStatus() {

    if(!flagGet(COMMUNICATION_OK_FLAG)) {
        CommunicationHandler::announceDevice(deviceId);
    }
    else{
        if(flagGet(ERROR_FLAG)) {
            CommunicationHandler::propagateError(getErrorCode());
        }
        CommunicationHandler::updateJointStatus(
            Motor::getInstance().get_position_deg(),
            Motor::getInstance().get_actual_speed()
        );
    }
}

void jointMainTask(void *pvParameters) {
  (void)pvParameters;

  deviceId = (GPIOB->IDR >> 4) & 0x0F;
  deviceId += 1;   // Id 0 are reserved for Master

  CommunicationHandler::start(deviceId);

  Motor &motor = Motor::getInstance(pwm, adc, ADC_CHANNEL_1,
                                  ms1, ms2, ms3, dir);
  Control &control = Control::getInstance(motor);
  (void)control;


  //declare Communication Services

  CommunicationHandler::registerMasterService(CMD_ERROR, [](const CAN_Message*) {
      flagSet(ERROR_FLAG);
  });
  CommunicationHandler::registerMasterService(CMD_WATCHDOG, [](const CAN_Message*) {
      lastMasterMsg = xTaskGetTickCount();
  });
  CommunicationHandler::registerService(CMD_CALIBRATION, [](const CAN_Message* msg) {
      lastMasterMsg = xTaskGetTickCount();
      uint16_t pos, pos_min, pos_max;
      int16_t ratio;
      memcpy(&pos,      &msg->data[0], sizeof(uint16_t));
      memcpy(&ratio,    &msg->data[2], sizeof(int16_t));
      memcpy(&pos_min,  &msg->data[4], sizeof(uint16_t));
      memcpy(&pos_max,  &msg->data[6], sizeof(uint16_t));
      Motor::getInstance().calibration(pos, ratio, pos_min, pos_max);
      printf("Calibration: Pos = %.2f deg, Ratio = %.2f, Min = %.2f deg, Max = %.2f deg\n",
             (float)pos / 100.0f, (float)ratio / 100.0f,
             (float)pos_min / 100.0f, (float)pos_max / 100.0f);
      flagSet(COMMUNICATION_OK_FLAG);
  });

  CommunicationHandler::registerService(CMD_START, [](const CAN_Message* ) {
        lastMasterMsg = xTaskGetTickCount();
        CommunicationHandler::registerService(CMD_MOVE_TARGET, [](const CAN_Message* msg) {
            lastMasterMsg = xTaskGetTickCount();
            float pos, speed;
            memcpy(&pos, &msg->data[0], sizeof(float));
            memcpy(&speed, &msg->data[4], sizeof(float));
            float pmin = Motor::getInstance().get_pos_min();
            float pmax = Motor::getInstance().get_pos_max();
            if (Protections::checkOutOfRange(pos, pmin, pmax)) {
                printf("Out of range: %.2f deg (min %.2f, max %.2f)\n", pos, pmin, pmax);
                return;
            }
            Control::getInstance().Move(pos, speed);
            printf("Move: Position = %.2f deg, Speed = %.2f deg/s\n", pos, speed);
        });
  });

//   CommunicationHandler::registerService(CMD_MOVE_TARGET, [](const CAN_Message* msg) {
//       float pos, speed;
//       memcpy(&pos, &msg->data[0], sizeof(float));
//       memcpy(&speed, &msg->data[4], sizeof(float));
//       Control::getInstance().Move(pos, speed);
//       printf("Move: Position = %.2f deg, Speed = %.2f deg/s\n", pos, speed);
//   });




  StateMachine &sm = StateMachine::get();

  lastMasterMsg = xTaskGetTickCount();

  for (;;) {
    sm.update();
    publishJointStatus();

    if (flagGet(COMMUNICATION_OK_FLAG) && !flagGet(ERROR_FLAG)) {
        if (Protections::checkCommunication(lastMasterMsg, xTaskGetTickCount()))
            printf("Master communication lost\n");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    printf("Position: %.2f deg | Speed: %.2f deg/s | Angle: %.2f deg | raw: %lu\n",
           motor.get_position_deg(), motor.get_actual_speed(),
           motor.get_angle_deg(), (unsigned long)motor.get_position_raw());
  }
}
