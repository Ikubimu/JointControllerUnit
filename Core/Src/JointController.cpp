#include "JointController.hpp"
#include "CommunicationHandler.hpp"
#include "Motor.hpp"
#include "Control.hpp"
#include "Peripherals.hpp"
#include "StateMachine/StateMachine.hpp"
#include "FlagUtils.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

uint8_t deviceId = 0;

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
  CommunicationHandler::registerService(CMD_CALIBRATION, [](const CAN_Message* msg) {
      float pos, speed;
      memcpy(&pos, &msg->data[0], sizeof(float));
      memcpy(&speed, &msg->data[4], sizeof(float));
      Motor::getInstance().calibration(pos, speed);
      printf("Calibration: Position = %.2f deg, Speed = %.2f deg/s\n", pos, speed);
      flagSet(COMMUNICATION_OK_FLAG);
  });

  CommunicationHandler::registerService(CMD_START, [](const CAN_Message* ) {
        CommunicationHandler::registerService(CMD_MOVE_TARGET, [](const CAN_Message* msg) {
            float pos, speed;
            memcpy(&pos, &msg->data[0], sizeof(float));
            memcpy(&speed, &msg->data[4], sizeof(float));
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

  for (;;) {
    sm.update();
    publishJointStatus();
    vTaskDelay(pdMS_TO_TICKS(1000));
    printf("Position: %.2f deg | Speed: %.2f deg/s | Angle: %.2f deg | raw: %lu\n",
           motor.get_position_deg(), motor.get_actual_speed(),
           motor.get_angle_deg(), (unsigned long)motor.get_position_raw());
  }
}
