#ifndef BSP_SDCARD_H
#define BSP_SDCARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    char card_type_str[16];
    char prod_name[6];
    uint32_t serial_number;
    uint8_t mfg_id;
    uint32_t block_count;
    uint32_t block_size;
    uint32_t capacity_mb;
    uint8_t card_version;
    uint8_t speed_class;
} BSP_SD_CardInfo_t;

typedef struct {
    bool initialized;
    bool wide_bus_4bit;
    uint32_t sdmmc_clk_hz;
    BSP_SD_CardInfo_t card_info;
    bool sector0_read_ok;
    bool has_mbr_signature;
    bool sector_rw_test_ok;
    uint32_t last_error_code;
} BSP_SD_DiagStatus_t;

/**
 * @brief Initialize SDMMC1 peripheral and configure PLL2 clock
 * @return true on success, false if card detection or init fails
 */
bool BSP_SD_Init(void);

/**
 * @brief De-initialize SDMMC1 peripheral
 */
void BSP_SD_DeInit(void);

/**
 * @brief Run full Phase 1 diagnostic test (Card Info, Sector 0 read, non-destructive R/W test)
 * @param diag Pointer to diagnostic status structure
 * @return true if card passed all tests
 */
bool BSP_SD_RunSelfTest(BSP_SD_DiagStatus_t *diag);

/**
 * @brief Read single 512-byte block in polling mode
 */
bool BSP_SD_ReadSector(uint32_t block_addr, uint8_t *buf);

/**
 * @brief Write single 512-byte block in polling mode
 */
bool BSP_SD_WriteSector(uint32_t block_addr, const uint8_t *buf);

/**
 * @brief Get human-readable error description
 */
const char *BSP_SD_GetErrorString(uint32_t error_code);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SDCARD_H */

