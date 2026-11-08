#include "FreeRTOS.h"
#include "bsp_board.h"
#include "task.h"

/* SysTick interrupt handler: feeds both HAL and FreeRTOS */
extern void xPortSysTickHandler(void);
void SysTick_Handler(void) {
  HAL_IncTick();
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    xPortSysTickHandler();
  }
}

/* --- Task 1: Blinks LED D2 (PA1) every 500ms --- */
static void Task_Blink(void *pvParameters) {
  (void)pvParameters;
  while (1) {
    BSP_LED_Toggle();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

/* --- Task 2: Monitors User Button K1 (PE3) in parallel --- */
static void Task_Button(void *pvParameters) {
  (void)pvParameters;
  while (1) {
    if (BSP_Button_K1_Read()) {
      BSP_LED_On();
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

int main(void) {

  // 1. Initialize HAL and Hardware
  HAL_Init();
  BSP_LED_Init();
  BSP_Buttons_Init();

  // 2. Set NVIC Priority Grouping to 4 bits (Required by FreeRTOS Cortex-M
  // port)
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  // 3. Create Task 1 (Blink)
  xTaskCreate(Task_Blink,  /* Task function */
              "BlinkTask", /* Name for debugging */
              256,         /* Stack size in words (1024 bytes) */
              NULL,        /* Parameter passed to task */
              2,   /* Priority (1 = low, higher numbers = higher priority) */
              NULL /* Task handle */
  );

  // 4. Create Task 2 (Button Monitor)
  xTaskCreate(Task_Button, "ButtonTask", 256, NULL, 1, NULL);

  // 5. Start the FreeRTOS Scheduler (never returns!)
  vTaskStartScheduler();

  while (1) {
  }
}