#include "JointController.hpp"
#include "Peripherals.hpp"

void vTaskCAN(void *pvParameters) {
  (void)pvParameters;
  uint32_t counter = 0;
  for (;;) {
    counter++;
    canMsg.data[0] = 0xAA;
    canMsg.data[1] = 0xBB;
    canMsg.data[2] = 0xCC;
    canMsg.data[3] = 0xDD;
    canMsg.data[4] = 0x11;
    canMsg.data[5] = 0x22;
    canMsg.data[6] = 0x33;
    canMsg.data[7] = 0x44;

    if (can.write_message(&canMsg))
      printf("CAN TX: ID=0x%03lX Count=%lu\r\n", canMsg.id, counter);
    else
      printf("CAN TX ERROR\r\n");

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

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
  can.start(0, 0);
  canMsg.id = 0x123;
  canMsg.dlc = 8;
  canMsg.is_extended = false;

  StateMachine &sm = StateMachine::get();

  for (;;) {
    sm.update();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
