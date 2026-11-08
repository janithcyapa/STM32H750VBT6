#include "FreeRTOS.h"
#include "bsp_board.h"
#include "bsp_qspi.h"
#include "task.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

extern void xPortSysTickHandler(void);
void SysTick_Handler(void) {
  HAL_IncTick();
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    xPortSysTickHandler();
  }
}

static void Task_BlinkAndUSB(void *pvParameters) {
  (void)pvParameters;
  uint32_t counter = 0;
  char msg[64];

  while (1) {
    BSP_LED_Toggle();

    // Print counter message every second over USB!
    int len = snprintf(msg, sizeof(msg), "[DevEBox H750] Uptime: %lu s\r\n",
                       counter++);
    CDC_Transmit_FS((uint8_t *)msg, len);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main(void) {
  HAL_Init();
  BSP_LED_Init();

  // Initialize USB CDC (Virtual COM Port)
  MX_USB_DEVICE_Init();

  // Start FreeRTOS
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  xTaskCreate(Task_BlinkAndUSB, "USBTask", 512, NULL, 2, NULL);
  vTaskStartScheduler();

  while (1) {
  }
}