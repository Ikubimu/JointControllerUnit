#include "JointController.hpp"
#include "CommunicationHandler.hpp"
#include "Motor.hpp"
#include "Peripherals.hpp"
#include "StateMachine/StateMachine.hpp"
#include "FlagUtils.hpp"
#include "FreeRTOS.h"
#include "task.h"

void vTaskADC(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    uint32_t adc0 = adc.read_channel(ADC_CHANNEL_1);
    uint32_t adc1 = adc.read_channel(ADC_CHANNEL_5);
    printf("ADC0: %lu | ADC1: %lu\r\n", adc0, adc1);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

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

void jointMainTask(void *pvParameters) {
  (void)pvParameters;

  CommunicationHandler::registerService(0x0001, [](const CAN_Message*) {
      flagSet(ERROR_FLAG);
  });
  CommunicationHandler::registerService(0x0101, [](const CAN_Message*) {
      flagSet(COMMUNICATION_OK_FLAG);
  });
  CommunicationHandler::start(0x01);

  Motor &motor = Motor::getInstance(pwm, adc, ADC_CHANNEL_1,
                                  ms1, ms2, ms3, dir);
  StateMachine &sm = StateMachine::get();

  for (;;) {
    sm.update();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
