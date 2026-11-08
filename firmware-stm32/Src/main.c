#include "FreeRTOS.h"
#include "bsp_board.h"
#include "bsp_qspi.h"
#include "task.h"

extern void xPortSysTickHandler(void);
void SysTick_Handler(void) {
  HAL_IncTick();
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    xPortSysTickHandler();
  }
}

static void Task_Blink(void *pvParameters) {
  (void)pvParameters;
  while (1) {
    BSP_LED_Toggle();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

int main(void) {
  HAL_Init();
  BSP_LED_Init();

  // 1. Initialize External QSPI Flash
  BSP_QSPI_Init();

  // 2. Read JEDEC ID from the W25Q64JV chip
  uint32_t flash_id = BSP_QSPI_ReadID();

  // 3. Verify the ID
  if (flash_id == W25Q64JV_EXPECTED_ID) // 0xEF4017
  {
    // Success: 3 Fast Blinks to confirm External Flash works!
    for (int i = 0; i < 3; i++) {
      BSP_LED_On();
      HAL_Delay(100);
      BSP_LED_Off();
      HAL_Delay(100);
    }

    // Put Flash into Memory-Mapped Mode (Available at 0x90000000)
    BSP_QSPI_EnableMemoryMappedMode();
  } else {
    // Error: Keep LED solid ON
    BSP_LED_On();
    while (1) {
    }
  }

  // Start FreeRTOS
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  xTaskCreate(Task_Blink, "BlinkTask", 256, NULL, 2, NULL);
  vTaskStartScheduler();

  while (1) {
  }
}