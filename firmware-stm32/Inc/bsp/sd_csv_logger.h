#ifndef SD_CSV_LOGGER_H
#define SD_CSV_LOGGER_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Test and initialize FAT32 filesystem on SD card
 * @return true if FAT32 partition mounted successfully
 */
bool SD_CSV_Logger_Init(void);

/**
 * @brief Start a new CSV logging session with auto-incremented filename (e.g. LOG0001.CSV)
 * @param out_filename Buffer to store chosen filename (at least 16 bytes)
 * @param filename_size Size of out_filename buffer
 * @param out_log_num Pointer to store the numeric index (e.g. 1)
 * @return true if file created and header written
 */
bool SD_CSV_Logger_Start(char *out_filename, size_t filename_size, uint16_t *out_log_num);

/**
 * @brief Write a CSV record line to the active file
 * @param record Formatted string terminating in \r\n
 * @return true on success
 */
bool SD_CSV_Logger_WriteRecord(const char *record);

/**
 * @brief Stop logging, flush all buffers, commit file size and unmount
 * @param out_records Total records written in this session
 * @return true on success
 */
bool SD_CSV_Logger_Stop(uint32_t *out_records);

/**
 * @brief Check if a logging session is currently active
 */
bool SD_CSV_Logger_IsActive(void);

#endif // SD_CSV_LOGGER_H

