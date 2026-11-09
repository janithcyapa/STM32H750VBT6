#include "sd_csv_logger.h"
#include "ff.h"
#include "usbd_cdc_if.h"
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

    // 4. Write CSV header line with complete per-anchor telemetry fields
    const char *header =
        "timestamp_ms,ax,ay,az,gx,gy,gz,mx,my,mz,temp_c,press_hpa,alt_m,uav_id,"
        "a1_dist_cm,a1_rssi,a1_rtt_ps,a1_t1,a1_t2,a1_t3,a1_t4,a1_ppm,"
        "a2_dist_cm,a2_rssi,a2_rtt_ps,a2_t1,a2_t2,a2_t3,a2_t4,a2_ppm,"
        "a3_dist_cm,a3_rssi,a3_rtt_ps,a3_t1,a3_t2,a3_t3,a3_t4,a3_ppm,"
        "a4_dist_cm,a4_rssi,a4_rtt_ps,a4_t1,a4_t2,a4_t3,a4_t4,a4_ppm,"
        "a5_dist_cm,a5_rssi,a5_rtt_ps,a5_t1,a5_t2,a5_t3,a5_t4,a5_ppm\r\n";
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

bool SD_CSV_ListFiles(void)
{
    if (s_is_logging) {
        const char *busy = "[BUSY_RECORDING]\r\n";
        CDC_Transmit_FS_Blocking((const uint8_t *)busy, (uint16_t)strlen(busy), 100);
        return false;
    }

    FRESULT res = f_mount(&s_fatfs, "", 1);
    if (res != FR_OK) {
        const char *err = "[ERR_MOUNT_FAILED]\r\n";
        CDC_Transmit_FS_Blocking((const uint8_t *)err, (uint16_t)strlen(err), 100);
        return false;
    }

    FF_DIR dir;
    res = f_opendir(&dir, "");
    if (res != FR_OK) {
        const char *err = "[ERR_OPEN_DIR]\r\n";
        CDC_Transmit_FS_Blocking((const uint8_t *)err, (uint16_t)strlen(err), 100);
        f_mount(NULL, "", 0);
        return false;
    }

    const char *start = "[LIST_BEGIN]\r\n";
    CDC_Transmit_FS_Blocking((const uint8_t *)start, (uint16_t)strlen(start), 100);

    FILINFO fno;
    char line[64];
    while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0) {
        if (!(fno.fattrib & AM_DIR)) {
            // Check for .CSV extension
            char *dot = strrchr(fno.fname, '.');
            if (dot && (strcasecmp(dot, ".CSV") == 0)) {
                int n = snprintf(line, sizeof(line), "FILE:%s,%lu\r\n", fno.fname, (unsigned long)fno.fsize);
                CDC_Transmit_FS_Blocking((const uint8_t *)line, (uint16_t)n, 100);
            }
        }
    }

    f_closedir(&dir);
    f_mount(NULL, "", 0);

    const char *end = "[LIST_END]\r\n";
    CDC_Transmit_FS_Blocking((const uint8_t *)end, (uint16_t)strlen(end), 100);
    return true;
}

bool SD_CSV_DownloadFile(const char *filename)
{
    if (s_is_logging) {
        const char *busy = "[BUSY_RECORDING]\r\n";
        CDC_Transmit_FS_Blocking((const uint8_t *)busy, (uint16_t)strlen(busy), 100);
        return false;
    }

    if (!filename || filename[0] == '\0') {
        const char *err = "[ERR_INVALID_NAME]\r\n";
        CDC_Transmit_FS_Blocking((const uint8_t *)err, (uint16_t)strlen(err), 100);
        return false;
    }

    FRESULT res = f_mount(&s_fatfs, "", 1);
    if (res != FR_OK) {
        const char *err = "[ERR_MOUNT_FAILED]\r\n";
        CDC_Transmit_FS_Blocking((const uint8_t *)err, (uint16_t)strlen(err), 100);
        return false;
    }

    FIL fil;
    res = f_open(&fil, filename, FA_READ);
    if (res != FR_OK) {
        char err[64];
        snprintf(err, sizeof(err), "[ERR_NOT_FOUND:%s]\r\n", filename);
        CDC_Transmit_FS_Blocking((const uint8_t *)err, (uint16_t)strlen(err), 100);
        f_mount(NULL, "", 0);
        return false;
    }

    uint32_t fsize = (uint32_t)f_size(&fil);
    char hdr[80];
    int hlen = snprintf(hdr, sizeof(hdr), "[START_FILE:%s,%lu]\r\n", filename, (unsigned long)fsize);
    CDC_Transmit_FS_Blocking((const uint8_t *)hdr, (uint16_t)hlen, 200);

    uint8_t chunk[512];
    UINT br = 0;
    while (!f_eof(&fil)) {
        res = f_read(&fil, chunk, sizeof(chunk), &br);
        if (res != FR_OK || br == 0) {
            break;
        }
        CDC_Transmit_FS_Blocking(chunk, (uint16_t)br, 500);
    }

    f_close(&fil);
    f_mount(NULL, "", 0);

    char foot[80];
    int flen = snprintf(foot, sizeof(foot), "\r\n[END_FILE:%s]\r\n", filename);
    CDC_Transmit_FS_Blocking((const uint8_t *)foot, (uint16_t)flen, 200);

    return true;
}

bool SD_CSV_DeleteFile(const char *filename)
{
    if (s_is_logging) {
        const char *busy = "[BUSY_RECORDING]\r\n";
        CDC_Transmit_FS_Blocking((const uint8_t *)busy, (uint16_t)strlen(busy), 100);
        return false;
    }

    FRESULT res = f_mount(&s_fatfs, "", 1);
    if (res != FR_OK) return false;

    res = f_unlink(filename);
    f_mount(NULL, "", 0);

    char resp[80];
    if (res == FR_OK) {
        snprintf(resp, sizeof(resp), "[OK_DELETED:%s]\r\n", filename);
    } else {
        snprintf(resp, sizeof(resp), "[ERR_DELETE:%s]\r\n", filename);
    }
    CDC_Transmit_FS_Blocking((const uint8_t *)resp, (uint16_t)strlen(resp), 100);
    return (res == FR_OK);
}
