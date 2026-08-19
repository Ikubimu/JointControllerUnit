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

void vTaskLED(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    led.toggle();
    out1.write(true);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskLEDPB0(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    ledPB0.toggle();
    vTaskDelay(pdMS_TO_TICKS(350));
  }
}

void publishJointStatus() {
    CommunicationHandler::updateJointStatus(
        Motor::getInstance().get_position_deg(),
        Motor::getInstance().get_actual_speed()
    );
}

void jointMainTask(void *pvParameters) {
  (void)pvParameters;

  uint8_t pinConfig = (GPIOB->IDR >> 4) & 0x0F;
  pinConfig += 1;   // Id 0 are reserved for Master

  flagSet(COMMUNICATION_OK_FLAG); // Just for testing, remove this line in production
  CommunicationHandler::start(pinConfig);

  Motor &motor = Motor::getInstance(pwm, adc, ADC_CHANNEL_1,
                                  ms1, ms2, ms3, dir);
  Control &control = Control::getInstance(motor);
  (void)control;


  //declare Communication Services

  CommunicationHandler::registerMasterService(0x01, [](const CAN_Message*) {
      flagSet(ERROR_FLAG);
  });
  CommunicationHandler::registerService(0x01, [](const CAN_Message*) {
      flagSet(COMMUNICATION_OK_FLAG);
  });

  CommunicationHandler::registerService(0x03, [](const CAN_Message* msg) {
      float pos, speed;
      memcpy(&pos, &msg->data[0], sizeof(float));
      memcpy(&speed, &msg->data[4], sizeof(float));
      Control::getInstance().Move(pos, speed);
      printf("Move: Position = %.2f deg, Speed = %.2f deg/s\n", pos, speed);
  });




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
