#include "sd_csv_logger.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>

static FATFS s_fatfs;
static FIL s_logfile;
static bool s_is_logging = false;
static uint32_t s_record_count = 0;
static char s_current_filename[16] = {0};

bool SD_CSV_Logger_Init(void)
{
    // Try mounting the SD card (drive "0:")
    FRESULT res = f_mount(&s_fatfs, "", 1);
    if (res == FR_OK) {
        // Unmount until next session
        f_mount(NULL, "", 0);
        return true;
    }
    return false;
}

bool SD_CSV_Logger_Start(char *out_filename, size_t filename_size, uint16_t *out_log_num)
{
    if (s_is_logging) {
        return false;
    }

    // 1. Mount FAT32 filesystem
    FRESULT res = f_mount(&s_fatfs, "", 1);
    if (res != FR_OK) {
        return false;
    }

    // 2. Scan root directory for next available LOGxxxx.CSV
    uint16_t log_num = 1;
    FILINFO fno;
    char fname[16];

    while (log_num < 10000) {
        snprintf(fname, sizeof(fname), "LOG%04u.CSV", log_num);
        res = f_stat(fname, &fno);
        if (res == FR_NO_FILE) {
            // File does not exist yet -> use this filename
            break;
        } else if (res != FR_OK) {
            // Unexpected filesystem error
            f_mount(NULL, "", 0);
            return false;
        }
        log_num++;
    }

    if (log_num >= 10000) {
        // Rolled over 9999 files
        f_mount(NULL, "", 0);
        return false;
    }

    strncpy(s_current_filename, fname, sizeof(s_current_filename) - 1);
    s_current_filename[sizeof(s_current_filename) - 1] = '\0';

    // 3. Create and open the file for writing
    res = f_open(&s_logfile, s_current_filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        f_mount(NULL, "", 0);
        return false;
    }

    // 4. Write CSV header line
    const char *header = "timestamp_ms,ax,ay,az,gx,gy,gz,mx,my,mz,temp_c,press_hpa,alt_m,telemetry\r\n";
    UINT bw = 0;
    res = f_write(&s_logfile, header, (UINT)strlen(header), &bw);
    if (res != FR_OK || bw != (UINT)strlen(header)) {
        f_close(&s_logfile);
        f_mount(NULL, "", 0);
        return false;
    }

    f_sync(&s_logfile);

    s_is_logging = true;
    s_record_count = 0;

    if (out_filename && filename_size > 0) {
        strncpy(out_filename, s_current_filename, filename_size - 1);
        out_filename[filename_size - 1] = '\0';
    }
    if (out_log_num) {
        *out_log_num = log_num;
    }

    return true;
}

bool SD_CSV_Logger_WriteRecord(const char *record)
{
    if (!s_is_logging || record == NULL) {
        return false;
    }

    UINT bw = 0;
    UINT len = (UINT)strlen(record);
    FRESULT res = f_write(&s_logfile, record, len, &bw);
    if (res != FR_OK) {
        return false;
    }

    s_record_count++;

    // Periodically commit data every 20 records (every 2 seconds at 10 Hz)
    if (s_record_count % 20 == 0) {
        f_sync(&s_logfile);
    }

    return true;
}

bool SD_CSV_Logger_Stop(uint32_t *out_records)
{
    if (!s_is_logging) {
        return false;
    }

    // Flush and commit directory entry & final file size
    f_close(&s_logfile);
    f_mount(NULL, "", 0);

    s_is_logging = false;

    if (out_records) {
        *out_records = s_record_count;
    }

    return true;
}

bool SD_CSV_Logger_IsActive(void)
{
    return s_is_logging;
}

