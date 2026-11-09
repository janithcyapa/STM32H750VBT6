#include "FreeRTOS.h"
#include "bmi160.h"
#include "bmp280.h"
#include "bsp_board.h"
#include "bsp_sdcard.h"
#include "sd_csv_logger.h"
#include "i2c_bus.h"
#include "qmc5883l.h"
#include "task.h"
#include "telemetry_uart.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <string.h>

extern void xPortSysTickHandler(void);
void SysTick_Handler(void) {
  HAL_IncTick();
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    xPortSysTickHandler();
  }
}

typedef struct {
  bool imu_ok;
  bool baro_ok;
  bool mag_ok;
  bool telem_ok;
  bool sd_ok;
  bool fatfs_ok;
  uint8_t imu_id;
  uint8_t baro_id;
  uint8_t mag_id;
  BSP_SD_DiagStatus_t sd_diag;
} SystemStatus_t;

static bool RunSystemVerification(SystemStatus_t *status) {
  memset(status, 0, sizeof(*status));

  // 1. Blink LED while verification is in progress
  for (int i = 0; i < 4; i++) {
    BSP_LED_On();
    vTaskDelay(pdMS_TO_TICKS(75));
    BSP_LED_Off();
    vTaskDelay(pdMS_TO_TICKS(75));
  }

  // 2. Initialize I2C1 Bus (PB8 SCL, PB9 SDA)
  I2C_Bus_Init();

  // 3. Verify BMI160 IMU over SPI1 (PA4..PA7)
  status->imu_ok = BMI160_Init();
  status->imu_id = BMI160_ReadChipID();
  if (status->imu_id != BMI160_CHIP_ID_VALUE) {
    status->imu_ok = false;
  }

  // 4. Verify BMP280 Barometer over I2C1 (PB8/PB9)
  status->baro_ok = BMP280_Init();
  status->baro_id = BMP280_ReadChipID();
  if (status->baro_id != BMP280_CHIP_ID_VALUE) {
    status->baro_ok = false;
  }

  // 5. Verify QMC5883L Magnetometer over I2C1 (PB8/PB9)
  status->mag_ok = QMC5883L_Init();
  status->mag_id = QMC5883L_ReadChipID();
  QMC5883L_Data_t dummy_mag = {0};
  if (!QMC5883L_Read(&dummy_mag)) {
    status->mag_ok = false;
  }

  // 6. Verify Telemetry UART4 (PD1 TX, PD0 RX)
  status->telem_ok = Telemetry_UART_IsReady();

  // 7. Verify SDMMC1 Hardware & FAT32 Filesystem
  status->sd_ok = BSP_SD_RunSelfTest(&status->sd_diag);
  if (status->sd_ok) {
    status->fatfs_ok = SD_CSV_Logger_Init();
  }

  // Overall Verification Result
  bool all_ok = status->imu_ok && status->baro_ok && status->mag_ok &&
                status->telem_ok && status->sd_ok && status->fatfs_ok;

  if (all_ok) {
    // Verification passed: turn LED OFF (stop blinking)
    BSP_LED_Off();
  }

  return all_ok;
}

static void PrintVerificationReport(const SystemStatus_t *status, bool all_ok) {
  char msg[512];
  int len = snprintf(
      msg, sizeof(msg),
      "\r\n===================================================\r\n"
      "      DevEBox STM32H750 Subsystem Verification     \r\n"
      "===================================================\r\n"
      "  [1] BMI160 IMU (SPI1)   : %s (ID: 0x%02X)\r\n"
      "  [2] BMP280 Baro (I2C1)  : %s (ID: 0x%02X)\r\n"
      "  [3] QMC5883 Mag (I2C1)  : %s (ID: 0x%02X)\r\n"
      "  [4] TELEMETRY (UART4)   : %s (PD1:TX, PD0:RX)\r\n"
      "  [5] SD CARD (FAT32)     : %s (%lu MB, MBR: %s, FS: %s)\r\n",
      status->imu_ok ? "PASS" : "FAIL", status->imu_id,
      status->baro_ok ? "PASS" : "FAIL", status->baro_id,
      status->mag_ok ? "PASS" : "FAIL", status->mag_id,
      status->telem_ok ? "PASS" : "FAIL",
      status->sd_ok ? "PASS" : "FAIL",
      (unsigned long)status->sd_diag.card_info.capacity_mb,
      status->sd_diag.has_mbr_signature ? "OK" : "RAW",
      status->fatfs_ok ? "FAT32 OK" : "NO FAT32");
  CDC_Transmit_FS((uint8_t *)msg, (uint16_t)len);
  vTaskDelay(pdMS_TO_TICKS(10));

  len = snprintf(
      msg, sizeof(msg),
      "---------------------------------------------------\r\n"
      "  OVERALL STATUS          : %s\r\n"
      "  LED STATUS              : %s\r\n"
      "---------------------------------------------------\r\n"
      "  [Button K1 (PE3)]       : Re-run System Verification\r\n"
      "  [Button K2 (PC5/PA0)]   : Toggle 10 Hz Recording & CSV Log\r\n"
      "===================================================\r\n\r\n",
      all_ok ? "ALL SYSTEMS VERIFIED OK -> IDLE"
             : "VERIFICATION FAILED (Check hardware/SD)",
      all_ok ? "OFF (Idle)" : "BLINKING (Error Indicator)");
  CDC_Transmit_FS((uint8_t *)msg, (uint16_t)len);
}

static void Task_Main_App(void *pvParameters) {
  (void)pvParameters;

  SystemStatus_t status = {0};
  bool all_ok = false;
  bool is_recording = false;
  uint32_t record_seq = 0;
  char active_filename[16] = {0};
  uint16_t active_log_num = 0;

  BMI160_RawData_t accel = {0};
  BMI160_RawData_t gyro = {0};
  BMP280_Data_t baro = {0};
  QMC5883L_Data_t mag = {0};
  char telem_str[128] = {0};
  char console_line[320];
  char csv_line[256];

  // Wait 1.5 seconds for USB CDC host enumeration
  vTaskDelay(pdMS_TO_TICKS(1500));

  // Initial Verification on Boot
  all_ok = RunSystemVerification(&status);
  PrintVerificationReport(&status, all_ok);

  uint8_t k1_prev = 0;
  uint8_t k2_prev = 0;
  uint32_t err_blink_tick = 0;

  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    // 10 Hz Task Loop (100 ms period)
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));

    // Sample User Pushbuttons
    uint8_t k1_curr = BSP_Button_K1_Read();
    uint8_t k2_curr = BSP_Button_K2_Read();

    bool k1_pressed = (k1_curr == 1 && k1_prev == 0);
    bool k2_pressed = (k2_curr == 1 && k2_prev == 0);
    k1_prev = k1_curr;
    k2_prev = k2_curr;

    // Button K1: Re-run verification
    if (k1_pressed) {
      if (is_recording) {
        // Automatically stop recording first if running
        uint32_t recs = 0;
        SD_CSV_Logger_Stop(&recs);
        is_recording = false;
        char stop_msg[128];
        int sl = snprintf(stop_msg, sizeof(stop_msg),
                          "\r\n[LOG] Recording stopped for verification (Saved %lu records in %s).\r\n",
                          (unsigned long)recs, active_filename);
        CDC_Transmit_FS((uint8_t *)stop_msg, (uint16_t)sl);
      }

      all_ok = RunSystemVerification(&status);
      PrintVerificationReport(&status, all_ok);
      xLastWakeTime = xTaskGetTickCount();
      continue;
    }

    // Button K2: Toggle 10 Hz Recording & SD CSV Log
    if (k2_pressed) {
      if (!is_recording) {
        if (SD_CSV_Logger_Start(active_filename, sizeof(active_filename), &active_log_num)) {
          is_recording = true;
          record_seq = 0;
          BSP_LED_On(); // Solid LED ON indicates active recording
          char start_msg[192];
          int sl = snprintf(start_msg, sizeof(start_msg),
                            "\r\n>>> [RECORDING STARTED] File: %s (#%u) <<<\r\n"
                            "Streaming 10 Hz data to console and writing CSV to SD card...\r\n"
                            "Press K2 again to STOP recording and finalize file.\r\n\r\n",
                            active_filename, active_log_num);
          CDC_Transmit_FS((uint8_t *)start_msg, (uint16_t)sl);
        } else {
          char err_msg[160];
          int el = snprintf(err_msg, sizeof(err_msg),
                            "\r\n[ERROR] Failed to create CSV file on SD card! Check FAT32 format.\r\n");
          CDC_Transmit_FS((uint8_t *)err_msg, (uint16_t)el);
        }
      } else {
        // Stop recording and close file
        uint32_t total_recs = 0;
        SD_CSV_Logger_Stop(&total_recs);
        is_recording = false;

        char stop_msg[220];
        int sl = snprintf(stop_msg, sizeof(stop_msg),
                          "\r\n>>> [RECORDING STOPPED] <<<\r\n"
                          "File Saved          : %s\r\n"
                          "Total Records Saved : %lu\r\n"
                          "File flushed & committed. Safe to unplug SD card.\r\n"
                          "Press K2 to start next recording session.\r\n\r\n",
                          active_filename, (unsigned long)total_recs);
        CDC_Transmit_FS((uint8_t *)stop_msg, (uint16_t)sl);

        // Restore LED state
        if (all_ok) {
          BSP_LED_Off();
        }
      }
    }

    // Execution branch based on recording state
    if (is_recording) {
      record_seq++;
      uint32_t now_ms = HAL_GetTick();

      // 1. Read IMU (Accel & Gyro)
      BMI160_ReadRaw(&accel, &gyro);

      // 2. Read Barometer (Temp, Pressure, Altitude)
      BMP280_Read(&baro);

      // 3. Read Magnetometer (RAW X, Y, Z)
      QMC5883L_Read(&mag);

      // 4. Retrieve and parse latest Telemetry UART4 line into AP1..AP5
      uint32_t age_ms = 0;
      Telemetry_AP_Data_t ap_data = {0};
      if (Telemetry_UART_GetLatestPacket(telem_str, sizeof(telem_str), &age_ms)) {
        size_t tlen = strlen(telem_str);
        while (tlen > 0 && (telem_str[tlen - 1] == '\r' || telem_str[tlen - 1] == '\n')) {
          telem_str[--tlen] = '\0';
        }
        Telemetry_ParseAPs(telem_str, &ap_data);
      } else {
        strncpy(telem_str, "NO_DATA", sizeof(telem_str) - 1);
      }

      // Format human-readable AP summary for console logger
      char ap_console[96] = {0};
      int ap_len = 0;
      for (int i = 0; i < 5; i++) {
        if (ap_data.valid[i]) {
          ap_len += snprintf(ap_console + ap_len, sizeof(ap_console) - ap_len,
                             "AP%d:[R:%d F:%ld] ", i + 1, ap_data.rssi[i], (long)ap_data.ftm[i]);
        }
      }
      if (ap_len == 0) {
        strncpy(ap_console, "NO_AP", sizeof(ap_console) - 1);
      }

      // 5. Output ONE unified line per time step to USB CDC Logger
      int clen = snprintf(
          console_line, sizeof(console_line),
          "[REC #%4lu] T:%6lu | A:[%6d %6d %6d] | G:[%5d %5d %5d] | M:[%6d %6d %6d] | B:[%4.1fC %7.2fhPa %5.1fm] | %s\r\n",
          (unsigned long)record_seq, (unsigned long)now_ms,
          accel.x, accel.y, accel.z,
          gyro.x, gyro.y, gyro.z,
          mag.x, mag.y, mag.z,
          baro.temperature_c, baro.pressure_hpa, baro.altitude_m,
          ap_console);
      CDC_Transmit_FS((uint8_t *)console_line, (uint16_t)clen);

      // 6. Write CSV record with split AP1..AP5 RSSI & FTM columns to SD Card File
      char ap_csv[10][16] = {{0}};
      for (int i = 0; i < 5; i++) {
        if (ap_data.valid[i]) {
          snprintf(ap_csv[i * 2], sizeof(ap_csv[0]), "%d", ap_data.rssi[i]);
          snprintf(ap_csv[i * 2 + 1], sizeof(ap_csv[0]), "%ld", (long)ap_data.ftm[i]);
        }
      }

      int rlen = snprintf(
          csv_line, sizeof(csv_line),
          "%lu,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,"
          "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\r\n",
          (unsigned long)now_ms,
          accel.x, accel.y, accel.z,
          gyro.x, gyro.y, gyro.z,
          mag.x, mag.y, mag.z,
          baro.temperature_c, baro.pressure_hpa, baro.altitude_m,
          ap_csv[0], ap_csv[1],
          ap_csv[2], ap_csv[3],
          ap_csv[4], ap_csv[5],
          ap_csv[6], ap_csv[7],
          ap_csv[8], ap_csv[9]);
      (void)rlen;
      SD_CSV_Logger_WriteRecord(csv_line);

    } else {
      // Idle state: handle USB File Transfer commands from Laptop / Python script
      if (CDC_HasCommand()) {
        char cmd[128];
        if (CDC_GetCommand(cmd, sizeof(cmd))) {
          char *p = cmd;
          while (*p == ' ' || *p == '\t') p++;
          size_t clen_cmd = strlen(p);
          while (clen_cmd > 0 && (p[clen_cmd - 1] == ' ' || p[clen_cmd - 1] == '\r' || p[clen_cmd - 1] == '\n')) {
            p[--clen_cmd] = '\0';
          }

          if (strcasecmp(p, "LIST") == 0) {
            SD_CSV_ListFiles();
          } else if (strncasecmp(p, "GET ", 4) == 0) {
            const char *fname = p + 4;
            while (*fname == ' ') fname++;
            SD_CSV_DownloadFile(fname);
          } else if (strncasecmp(p, "DEL ", 4) == 0) {
            const char *fname = p + 4;
            while (*fname == ' ') fname++;
            SD_CSV_DeleteFile(fname);
          } else if (strcasecmp(p, "STATUS") == 0) {
            char smsg[64];
            snprintf(smsg, sizeof(smsg), "[STATUS:IDLE,VERIFIED:%s]\r\n", all_ok ? "OK" : "ERR");
            CDC_Transmit_FS_Blocking((const uint8_t *)smsg, (uint16_t)strlen(smsg), 100);
          }
        }
      }

      if (!all_ok) {
        // Verification failed: keep LED blinking (toggle every 250ms)
        if ((HAL_GetTick() - err_blink_tick) >= 250) {
          err_blink_tick = HAL_GetTick();
          BSP_LED_Toggle();
        }
      } else {
        // System healthy & idle: LED OFF
        BSP_LED_Off();
      }
    }
  }
}

int main(void) {
  HAL_Init();

  BSP_LED_Init();
  BSP_Buttons_Init();

  // Initialize Telemetry UART (UART4 on PD1 TX, PD0 RX at 115200 baud)
  Telemetry_UART_Init(115200);

  // Initialize Native USB CDC Virtual COM Port
  MX_USB_DEVICE_Init();

  // Create FreeRTOS tasks
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  xTaskCreate(Task_Main_App, "MainApp", 1024, NULL, 2, NULL);
  xTaskCreate(Task_Telemetry_Bridge, "TelemBridge", 512, NULL, 2, NULL);
  vTaskStartScheduler();

  while (1) {
  }
}
