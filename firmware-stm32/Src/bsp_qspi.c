#include "bsp_qspi.h"

static QSPI_HandleTypeDef hqspi;

/* W25Q64JV SPI Commands */
#define CMD_READ_JEDEC_ID      0x9F
#define CMD_WRITE_ENABLE       0x06
#define CMD_READ_STATUS_REG1   0x05
#define CMD_FAST_READ_QUAD_IO  0xEB

uint8_t BSP_QSPI_Init(void)
{
    // 1. Enable Clocks
    __HAL_RCC_QSPI_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    // 2. Configure GPIO Pins for DevEBox H7XX-M
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;

    // CLK: PB2 (AF9)
    GPIO_InitStruct.Pin       = GPIO_PIN_2;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // CS / NCS: PB6 (AF10)
    GPIO_InitStruct.Pin       = GPIO_PIN_6;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // IO0 (PD11), IO1 (PD12), IO3 (PD13): (AF9)
    GPIO_InitStruct.Pin       = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    // IO2: PE2 (AF9)
    GPIO_InitStruct.Pin       = GPIO_PIN_2;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    // 3. Configure QUADSPI Peripheral
    hqspi.Instance                = QUADSPI;
    hqspi.Init.ClockPrescaler     = 3; // QSPI Clock = 64MHz / (3 + 1) = 16MHz (Safe speed)
    hqspi.Init.FifoThreshold      = 4;
    hqspi.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    hqspi.Init.FlashSize          = 22; // 2^(22+1) = 8 MBytes (23 address bits)
    hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_4_CYCLE;
    hqspi.Init.ClockMode          = QSPI_CLOCK_MODE_0;
    hqspi.Init.FlashID            = QSPI_FLASH_ID_1;
    hqspi.Init.DualFlash          = QSPI_DUALFLASH_DISABLE;

    return (HAL_QSPI_Init(&hqspi) == HAL_OK) ? 0 : 1;
}

uint32_t BSP_QSPI_ReadID(void)
{
    QSPI_CommandTypeDef sCommand = {0};
    uint8_t id_buf[3] = {0};

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = CMD_READ_JEDEC_ID;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 0;
    sCommand.NbData            = 3;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return 0;
    }

    if (HAL_QSPI_Receive(&hqspi, id_buf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return 0;
    }

    // Combine Manufacturer (0xEF), Memory Type (0x40), Capacity (0x17)
    return (uint32_t)((id_buf[0] << 16) | (id_buf[1] << 8) | id_buf[2]);
}

uint8_t BSP_QSPI_EnableMemoryMappedMode(void)
{
    QSPI_CommandTypeDef sCommand = {0};
    QSPI_MemoryMappedTypeDef sMemMappedCfg = {0};

    // Fast Read Quad I/O (0xEB)
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = CMD_FAST_READ_QUAD_IO;
    sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.DummyCycles       = 6; // Standard dummy cycles for 0xEB on W25Q64
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

    return (HAL_QSPI_MemoryMapped(&hqspi, &sCommand, &sMemMappedCfg) == HAL_OK) ? 0 : 1;
}