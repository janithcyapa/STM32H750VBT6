/* Low level disk I/O module for FatFs linking to STM32 BSP SD Card */

#include "diskio.h"
#include "ff.h"
#include "bsp_sdcard.h"
#include <string.h>

#define DEV_SD 0

DSTATUS disk_status(BYTE pdrv) {
    if (pdrv != DEV_SD) return STA_NOINIT;
    return 0; // OK
}

DSTATUS disk_initialize(BYTE pdrv) {
    if (pdrv != DEV_SD) return STA_NOINIT;
    if (!BSP_SD_Init()) {
        return STA_NOINIT;
    }
    return 0; // OK
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
    if (pdrv != DEV_SD || buff == NULL || count == 0) return RES_PARERR;
    for (UINT i = 0; i < count; i++) {
        if (!BSP_SD_ReadSector((uint32_t)(sector + i), buff + (i * 512))) {
            return RES_ERROR;
        }
    }
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
    if (pdrv != DEV_SD || buff == NULL || count == 0) return RES_PARERR;
    for (UINT i = 0; i < count; i++) {
        if (!BSP_SD_WriteSector((uint32_t)(sector + i), buff + (i * 512))) {
            return RES_ERROR;
        }
    }
    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    if (pdrv != DEV_SD) return RES_PARERR;

    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_COUNT:
            if (buff == NULL) return RES_PARERR;
            *(LBA_t *)buff = (LBA_t)BSP_SD_GetBlockCount();
            return RES_OK;
        case GET_SECTOR_SIZE:
            if (buff == NULL) return RES_PARERR;
            *(WORD *)buff = 512;
            return RES_OK;
        case GET_BLOCK_SIZE:
            if (buff == NULL) return RES_PARERR;
            *(DWORD *)buff = 1;
            return RES_OK;
        default:
            return RES_PARERR;
    }
}

DWORD get_fattime(void) {
    return ((DWORD)(2026 - 1980) << 25 | (DWORD)11 << 21 | (DWORD)9 << 16 | (DWORD)23 << 11 | (DWORD)55 << 5);
}

