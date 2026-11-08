#include "FreeRTOS.h"
#include "task.h"
#include "bsp_board.h"
#include "bsp_bmi160.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include <stdio.h>

extern void xPortSysTickHandler(void);
void SysTick_Handler(void) {
  HAL_IncTick();
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    xPortSysTickHandler();
  }
}

static void Task_IMU_Stream(void *pvParameters) {
  (void)pvParameters;
  char msg[256];
  BMI160_RawData_t accel = {0};
  BMI160_RawData_t gyro = {0};
  bool imu_ready = false;

  // Small delay for USB host to connect
  vTaskDelay(pdMS_TO_TICKS(1500));

  // Initialize BMI160 over SPI1
  imu_ready = BSP_BMI160_Init();
  uint8_t chip_id = BSP_BMI160_ReadChipID();

  if (imu_ready) {
    int len = snprintf(msg, sizeof(msg),
                       "\r\n========================================\r\n"
                       "[BMI160] Online! Chip ID: 0x%02X (Match 0xD1)\r\n"
                       "Streaming Accel (+/-4g) and Gyro (+/-2000dps)...\r\n"
                       "========================================\r\n", chip_id);
    CDC_Transmit_FS((uint8_t *)msg, len);
  } else {
    int len = snprintf(msg, sizeof(msg),
                       "\r\n[BMI160 ERROR] Init FAILED! Read ID: 0x%02X (Expected 0xD1)\r\n"
                       "Check SPI wiring: PA4(CS), PA5(SCK), PA6(MISO), PA7(MOSI)\r\n", chip_id);
    CDC_Transmit_FS((uint8_t *)msg, len);
  }

  while (1) {
    BSP_LED_Toggle();

    if (imu_ready) {
      BSP_BMI160_ReadRaw(&accel, &gyro);

      // Convert raw counts to approximate physical units:
      // +/-4g range: 8192 LSB/g -> (accel * 1000) / 8192 gives mg
      // +/-2000dps range: 16.4 LSB/(deg/s)
      int len = snprintf(msg, sizeof(msg),
                         "[IMU] A:[%6d %6d %6d] | G:[%6d %6d %6d]\r\n",
                         accel.x, accel.y, accel.z,
                         gyro.x, gyro.y, gyro.z);
      CDC_Transmit_FS((uint8_t *)msg, len);
    } else {
      // Retry init periodically if failed
      chip_id = BSP_BMI160_ReadChipID();
      int len = snprintf(msg, sizeof(msg),
                         "[BMI160] Retrying... ID read: 0x%02X (Expected: 0xD1)\r\n", chip_id);
      CDC_Transmit_FS((uint8_t *)msg, len);
      if (chip_id == 0xD1) {
        imu_ready = BSP_BMI160_Init();
      }
    }

    // Stream at 10 Hz (every 100ms) for readable terminal output
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

int main(void) {
  HAL_Init();
  BSP_LED_Init();

  // Initialize Native USB CDC Virtual COM Port
  MX_USB_DEVICE_Init();

  // Create FreeRTOS IMU streaming task
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  xTaskCreate(Task_IMU_Stream, "IMUTask", 512, NULL, 2, NULL);
  vTaskStartScheduler();

  while (1) {
  }
}
