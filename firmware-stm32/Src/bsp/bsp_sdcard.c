#include "bsp_sdcard.h"
#include <string.h>

static SD_HandleTypeDef hsd1;

// Internal buffers (aligned to 4 bytes for 32-bit FIFO access)
static uint8_t sector_backup[512] __attribute__((aligned(4)));
static uint8_t sector_test[512] __attribute__((aligned(4)));
static uint8_t sector_verify[512] __attribute__((aligned(4)));

void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
    if (hsd->Instance == SDMMC1) {
        // 1. Enable GPIO Clocks
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();

        // 2. Configure GPIOC pins:
        //    PC8  -> SDMMC1_D0
        //    PC9  -> SDMMC1_D1
        //    PC10 -> SDMMC1_D2
        //    PC11 -> SDMMC1_D3
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        //    PC12 -> SDMMC1_CK (No internal pull-up on clock line)
        GPIO_InitStruct.Pin = GPIO_PIN_12;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        // 3. Configure GPIOD pins:
        //    PD2  -> SDMMC1_CMD
        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        // 4. Enable SDMMC1 Clock
        __HAL_RCC_SDMMC1_CLK_ENABLE();
    }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef *hsd)
{
    if (hsd->Instance == SDMMC1) {
        __HAL_RCC_SDMMC1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
    }
}

static bool SD_ConfigureClock(void)
{
    // If PLL2 is already locked and running, avoid re-configuration which stalls the peripheral
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PLL2RDY)) {
        return true;
    }

    // Route PLL2R as SDMMC kernel clock.
    // HSI = 64 MHz -> PLL2M=8 (8MHz ref) -> PLL2N=50 (400MHz VCO) -> PLL2R=8 (50 MHz SDMMC_CK)
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC;
    PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL2;
    PeriphClkInitStruct.PLL2.PLL2M = 8;
    PeriphClkInitStruct.PLL2.PLL2N = 50;
    PeriphClkInitStruct.PLL2.PLL2P = 2;
    PeriphClkInitStruct.PLL2.PLL2Q = 2;
    PeriphClkInitStruct.PLL2.PLL2R = 8;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        return false;
    }

    // Bounded check for PLL2RDY
    uint32_t timeout = 200000;
    while (!__HAL_RCC_GET_FLAG(RCC_FLAG_PLL2RDY) && --timeout) ;

    return (timeout > 0);
}

static bool SD_WaitCardReady(uint32_t max_trials)
{
    uint32_t trials = 0;
    while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER) {
        if (++trials > max_trials) {
            return false;
        }
    }
    return true;
}

bool BSP_SD_Init(void)
{
    // 1. Configure Clock safely
    if (!SD_ConfigureClock()) {
        return false;
    }

    // 2. If previously initialized or in error state, de-init first
    if (hsd1.State != HAL_SD_STATE_RESET) {
        HAL_SD_DeInit(&hsd1);
    }

    // 3. Initialize SDMMC1 Instance
    hsd1.Instance = SDMMC1;
    hsd1.Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
    hsd1.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd1.Init.BusWide             = SDMMC_BUS_WIDE_1B;
    hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
    hsd1.Init.ClockDiv            = 4; // 50 MHz / (2 * 4) = 6.25 MHz (prevents FIFO overrun in polling mode)

    if (HAL_SD_Init(&hsd1) != HAL_OK) {
        return false;
    }

    // 4. Switch to 4-bit bus mode (fallback to 1-bit if not supported)
    HAL_SD_ConfigWideBusOperation(&hsd1, SDMMC_BUS_WIDE_4B);

    return true;
}

void BSP_SD_DeInit(void)
{
    HAL_SD_DeInit(&hsd1);
}

bool BSP_SD_ReadSector(uint32_t block_addr, uint8_t *buf)
{
    // Brief critical section: polling 512 bytes @ 6.25MHz takes ~80 microseconds.
    // Disabling interrupts prevents FreeRTOS SysTick or USB from causing FIFO Rx Overrun.
    __disable_irq();
    HAL_StatusTypeDef status = HAL_SD_ReadBlocks(&hsd1, buf, block_addr, 1, 1000);
    __enable_irq();

    if (status != HAL_OK) {
        return false;
    }
    return SD_WaitCardReady(50000);
}

bool BSP_SD_WriteSector(uint32_t block_addr, const uint8_t *buf)
{
    __disable_irq();
    HAL_StatusTypeDef status = HAL_SD_WriteBlocks(&hsd1, (uint8_t *)buf, block_addr, 1, 1000);
    __enable_irq();

    if (status != HAL_OK) {
        return false;
    }
    return SD_WaitCardReady(50000);
}

bool BSP_SD_RunSelfTest(BSP_SD_DiagStatus_t *diag)
{
    if (!diag) return false;
    memset(diag, 0, sizeof(*diag));

    // 1. Hardware Init
    if (!BSP_SD_Init()) {
        diag->last_error_code = HAL_SD_GetError(&hsd1);
        return false;
    }
    diag->initialized = true;
    diag->sdmmc_clk_hz = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SDMMC);

    // 2. Check 4-bit status
    diag->wide_bus_4bit = (hsd1.Init.BusWide == SDMMC_BUS_WIDE_4B);

    // 3. Query Card Info
    HAL_SD_CardInfoTypeDef card_info = {0};
    if (HAL_SD_GetCardInfo(&hsd1, &card_info) == HAL_OK) {
        diag->card_info.block_count = card_info.BlockNbr;
        diag->card_info.block_size  = card_info.BlockSize;
        diag->card_info.capacity_mb = (uint32_t)(((uint64_t)card_info.BlockNbr * card_info.BlockSize) / (1024ULL * 1024ULL));
        diag->card_info.card_version = card_info.CardVersion;
        diag->card_info.speed_class  = card_info.Class;

        switch (card_info.CardType) {
            case CARD_SDSC:
                strncpy(diag->card_info.card_type_str, "SDSC", sizeof(diag->card_info.card_type_str));
                break;
            case CARD_SDHC_SDXC:
                strncpy(diag->card_info.card_type_str, "SDHC/SDXC", sizeof(diag->card_info.card_type_str));
                break;
            case CARD_SECURED:
                strncpy(diag->card_info.card_type_str, "SECURED", sizeof(diag->card_info.card_type_str));
                break;
            default:
                strncpy(diag->card_info.card_type_str, "UNKNOWN", sizeof(diag->card_info.card_type_str));
                break;
        }
    }

    // 4. Query Card CID (Manufacturer, Name, Serial)
    HAL_SD_CardCIDTypeDef card_cid = {0};
    if (HAL_SD_GetCardCID(&hsd1, &card_cid) == HAL_OK) {
        diag->card_info.mfg_id = card_cid.ManufacturerID;
        diag->card_info.serial_number = card_cid.ProdSN;
        diag->card_info.prod_name[0] = (char)((card_cid.ProdName1 >> 24) & 0xFF);
        diag->card_info.prod_name[1] = (char)((card_cid.ProdName1 >> 16) & 0xFF);
        diag->card_info.prod_name[2] = (char)((card_cid.ProdName1 >> 8) & 0xFF);
        diag->card_info.prod_name[3] = (char)(card_cid.ProdName1 & 0xFF);
        diag->card_info.prod_name[4] = (char)card_cid.ProdName2;
        diag->card_info.prod_name[5] = '\0';
    }

    // 5. Test Reading Sector 0 (MBR or Boot Sector)
    if (BSP_SD_ReadSector(0, sector_backup)) {
        diag->sector0_read_ok = true;
        // Check for 0x55, 0xAA boot/MBR signature at byte 510-511
        if (sector_backup[510] == 0x55 && sector_backup[511] == 0xAA) {
            diag->has_mbr_signature = true;
        }
    } else {
        diag->last_error_code = HAL_SD_GetError(&hsd1);
    }

    // 6. Safe Non-Destructive Sector R/W Test:
    //    Choose block 2048 (beyond partition table).
    //    a. Read & backup original data.
    //    b. Write deterministic test pattern.
    //    c. Read back and verify match.
    //    d. Restore original data to preserve existing files/partition table.
    uint32_t test_block = 2048;
    if (test_block < diag->card_info.block_count) {
        if (BSP_SD_ReadSector(test_block, sector_backup)) {
            // Generate test pattern
            for (int i = 0; i < 512; i++) {
                sector_test[i] = (uint8_t)(i ^ 0xA5);
            }

            // Write pattern
            if (BSP_SD_WriteSector(test_block, sector_test)) {
                // Read back
                if (BSP_SD_ReadSector(test_block, sector_verify)) {
                    if (memcmp(sector_test, sector_verify, 512) == 0) {
                        diag->sector_rw_test_ok = true;
                    }
                }
            }

            // Restore original data
            BSP_SD_WriteSector(test_block, sector_backup);
        }
    }

    return (diag->initialized && diag->sector0_read_ok);
}

const char *BSP_SD_GetErrorString(uint32_t error_code)
{
    if (error_code == HAL_SD_ERROR_NONE) return "No Error";
    if (error_code & HAL_SD_ERROR_CMD_CRC_FAIL) return "Command CRC Fail";
    if (error_code & HAL_SD_ERROR_DATA_CRC_FAIL) return "Data CRC Fail";
    if (error_code & HAL_SD_ERROR_CMD_RSP_TIMEOUT) return "Command Response Timeout";
    if (error_code & HAL_SD_ERROR_DATA_TIMEOUT) return "Data Timeout";
    if (error_code & HAL_SD_ERROR_TX_UNDERRUN) return "Tx Underrun";
    if (error_code & HAL_SD_ERROR_RX_OVERRUN) return "Rx Overrun";
    if (error_code & HAL_SD_ERROR_ADDR_MISALIGNED) return "Address Misaligned";
    if (error_code & HAL_SD_ERROR_BLOCK_LEN_ERR) return "Block Length Error";
    if (error_code & HAL_SD_ERROR_WRITE_PROT_VIOLATION) return "Write Protect Violation";
    if (error_code & HAL_SD_ERROR_TIMEOUT) return "Timeout (No Card Detected)";
    if (error_code & HAL_SD_ERROR_PARAM) return "Invalid Parameter";
    return "Unknown/Hardware Error";
}
