#ifndef BSP_QSPI_H
#define BSP_QSPI_H

#include "stm32h7xx_hal.h"

/* Winbond W25Q64JV Identification */
#define W25Q64JV_FLASH_SIZE             0x800000 /* 8 MBytes */
#define W25Q64JV_EXPECTED_ID            0xEF4017 /* Winbond JEDEC ID */

/* QSPI Memory-Mapped Base Address */
#define QSPI_BASE_ADDR                  0x90000000

/* Public Functions */
uint8_t BSP_QSPI_Init(void);
uint32_t BSP_QSPI_ReadID(void);
uint8_t BSP_QSPI_EnableMemoryMappedMode(void);

#endif /* BSP_QSPI_H */