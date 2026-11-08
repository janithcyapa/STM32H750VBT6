#include "FreeRTOS.h"
#include "task.h"
#include "bsp_board.h"
#include "bmi160.h"
#include "i2c_bus.h"
#include "bmp280.h"
#include "qmc5883l.h"
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

static void Task_Sensors_Stream(void *pvParameters) {
  (void)pvParameters;
  char msg[384];

  BMI160_RawData_t accel = {0};
  BMI160_RawData_t gyro = {0};
  BMP280_Data_t baro = {0};
  QMC5883L_Data_t mag = {0};

  bool imu_ready = false;
  bool baro_ready = false;
  bool mag_ready = false;

  // Wait 1.5 seconds for USB CDC to connect on host PC
  vTaskDelay(pdMS_TO_TICKS(1500));

  // Banner
  int len = snprintf(msg, sizeof(msg),
                     "\r\n===================================================\r\n"
                     "  DevEBox STM32H750 Sensor Suite Bring-Up Console  \r\n"
                     "===================================================\r\n");
  CDC_Transmit_FS((uint8_t *)msg, len);
  vTaskDelay(pdMS_TO_TICKS(50));

  // 1. Initialize I2C1 Bus (PB8 SCL, PB9 SDA) & Run Scanner
  I2C_Bus_Init();
  char scan_buf[384];
  I2C_Bus_Scan(scan_buf, sizeof(scan_buf));
  CDC_Transmit_FS((uint8_t *)scan_buf, strlen(scan_buf));
  vTaskDelay(pdMS_TO_TICKS(50));

  // 2. Initialize BMI160 IMU over SPI1 (PA4..PA7)
  imu_ready = BMI160_Init();
  uint8_t imu_id = BMI160_ReadChipID();
  uint8_t imu_hal = BMI160_GetLastHALStatus();

  // 3. Initialize BMP280 Barometer over I2C1 (PB8/PB9)
  baro_ready = BMP280_Init();
  uint8_t baro_id = BMP280_ReadChipID();

  // 4. Initialize QMC5883L Magnetometer over I2C1 (PB8/PB9)
  mag_ready = QMC5883L_Init();
  uint8_t mag_id = QMC5883L_ReadChipID();

  // Print Sensor Status Summary
  len = snprintf(msg, sizeof(msg),
                 "\r\n--- Sensor Initialization Status ---\r\n"
                 "[1] BMI160 IMU  (SPI1): %s (ID: 0x%02X, HAL: %u)\r\n"
                 "[2] BMP280 Baro (I2C1): %s (ID: 0x%02X, Match 0x58)\r\n"
                 "[3] QMC5883 Mag (I2C1): %s (ID: 0x%02X, Addr 0x0D)\r\n"
                 "------------------------------------\r\n"
                 "Press Button K1 (PE3) at any time to re-scan I2C!\r\n"
                 "Starting live stream (10 Hz)...\r\n\r\n",
                 imu_ready ? "ONLINE" : "FAILED", imu_id, imu_hal,
                 baro_ready ? "ONLINE" : "FAILED", baro_id,
                 mag_ready ? "ONLINE" : "FAILED", mag_id);
  CDC_Transmit_FS((uint8_t *)msg, len);
  vTaskDelay(pdMS_TO_TICKS(100));

  uint32_t loop_count = 0;

  while (1) {
    BSP_LED_Toggle();
    loop_count++;

    // Read IMU
    if (imu_ready) {
      BMI160_ReadRaw(&accel, &gyro);
    } else if (loop_count % 20 == 0) {
      if (BMI160_ReadChipID() == 0xD1) imu_ready = BMI160_Init();
    }

    // Check Button K1 for manual re-scan
    if (BSP_Button_K1_Read()) {
      I2C2_Bus_Scan(scan_buf, sizeof(scan_buf));
      CDC_Transmit_FS((uint8_t *)scan_buf, strlen(scan_buf));
      baro_ready = BMP280_Init();
      mag_ready = QMC5883L_Init();
      vTaskDelay(pdMS_TO_TICKS(300));
    }

    // Read Barometer
    if (baro_ready) {
      BMP280_Read(&baro);
    } else if (loop_count % 30 == 0) {
      if (BMP280_ReadChipID() == 0x58) baro_ready = BMP280_Init();
    }

    // Read Magnetometer
    if (mag_ready) {
      QMC5883L_Read(&mag);
    } else if (loop_count % 30 == 0) {
      mag_ready = QMC5883L_Init();
    }

    // Stream Formatted Output:
    // Line 1: IMU Accel & Gyro
    // Line 2: Barometer & Magnetometer
    if (baro_ready && mag_ready) {
      len = snprintf(msg, sizeof(msg),
                     "[IMU]  A:[%6d %6d %6d] | G:[%6d %6d %6d]\r\n"
                     "[ENV]  Temp: %5.1f C | Press: %7.2f hPa | Alt: %6.1f m | Heading: %5.1f deg\r\n",
                     accel.x, accel.y, accel.z,
                     gyro.x, gyro.y, gyro.z,
                     baro.temperature_c, baro.pressure_hpa, baro.altitude_m,
                     mag.heading_deg);
    } else {
      // Diagnostic output when sensors are not yet ready
      len = snprintf(msg, sizeof(msg),
                     "[IMU]  A:[%6d %6d %6d] | G:[%6d %6d %6d]\r\n"
                     "[ENV]  Baro: %s (ID:0x%02X) | Mag: %s (ID:0x%02X) [Check PB8/SCL, PB9/SDA]\r\n",
                     accel.x, accel.y, accel.z,
                     gyro.x, gyro.y, gyro.z,
                     baro_ready ? "OK" : "NOT DETECTED", BMP280_ReadChipID(),
                     mag_ready ? "OK" : "NOT DETECTED", QMC5883L_ReadChipID());
    }
    CDC_Transmit_FS((uint8_t *)msg, len);

    // Stream at 10 Hz (every 100ms)
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

int main(void) {
  HAL_Init();
  BSP_LED_Init();
  BSP_Buttons_Init();

  // Initialize Native USB CDC Virtual COM Port
  MX_USB_DEVICE_Init();

  // Create FreeRTOS sensor streaming task
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  xTaskCreate(Task_Sensors_Stream, "SensorTask", 768, NULL, 2, NULL);
  vTaskStartScheduler();

  while (1) {
  }
}
