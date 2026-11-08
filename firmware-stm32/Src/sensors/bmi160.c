#include "bmi160.h"
#include <string.h>

static SPI_HandleTypeDef hspi1 = {0};
static uint8_t last_hal_status = 0;

static void BMI160_CS_Low(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
}

static void BMI160_CS_High(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

static uint8_t BMI160_ReadReg(uint8_t reg) {
    uint8_t tx[2] = { (uint8_t)(reg | 0x80), 0x00 };
    uint8_t rx[2] = { 0, 0 };

    BMI160_CS_Low();
    last_hal_status = HAL_SPI_TransmitReceive(&hspi1, tx, rx, 2, 50);
    BMI160_CS_High();

    return rx[1];
}

static void BMI160_WriteReg(uint8_t reg, uint8_t val) {
    uint8_t tx[2] = { (uint8_t)(reg & 0x7F), val };

    BMI160_CS_Low();
    HAL_SPI_Transmit(&hspi1, tx, 2, 50);
    BMI160_CS_High();
}

static void BMI160_ReadMulti(uint8_t start_reg, uint8_t *buf, uint16_t len) {
    uint8_t tx[16] = {0};
    uint8_t rx[16] = {0};

    if (len > 14) len = 14;
    tx[0] = (uint8_t)(start_reg | 0x80);

    BMI160_CS_Low();
    HAL_SPI_TransmitReceive(&hspi1, tx, rx, len + 1, 100);
    BMI160_CS_High();

    memcpy(buf, &rx[1], len);
}

bool BMI160_Init(void) {
    // 1. Configure Kernel Clock for SPI1/2/3
    // By default on STM32H7, SPI123 clock source is PLL1 (which is uninitialized).
    // We explicitly route HSI (64 MHz) via CKPER as the SPI123 kernel clock.
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI123 | RCC_PERIPHCLK_CKPER;
    PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_CLKP;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        return false;
    }

    // 2. Enable GPIOA and SPI1 Clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();

    // 3. Configure PA4 as Manual Chip Select (CS) - GPIO Output Push-Pull
    GPIO_InitTypeDef gpio_cs = {0};
    gpio_cs.Pin = GPIO_PIN_4;
    gpio_cs.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_cs.Pull = GPIO_NOPULL;
    gpio_cs.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio_cs);
    BMI160_CS_High();

    // 4. Configure PA5 (SCK), PA6 (MISO), PA7 (MOSI) as SPI1 AF5
    GPIO_InitTypeDef gpio_spi = {0};
    gpio_spi.Pin = GPIO_PIN_5 | GPIO_PIN_7;
    gpio_spi.Mode = GPIO_MODE_AF_PP;
    gpio_spi.Pull = GPIO_NOPULL;
    gpio_spi.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_spi.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &gpio_spi);

    // MISO with weak pull-up to prevent floating pin
    gpio_spi.Pin = GPIO_PIN_6;
    gpio_spi.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio_spi);

    // 5. Configure SPI1 Peripheral
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;   // Mode 0 (CPOL=0)
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;       // Mode 0 (CPHA=0)
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64; // 64 MHz / 64 = 1 MHz (safe for breadboard)
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;

    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        return false;
    }

    HAL_Delay(20);

    // 6. Switch BMI160 into SPI mode
    // Bosch BMI160 defaults to I2C at power-up.
    // A rising edge on CSB locks it into SPI mode.
    BMI160_CS_Low();
    HAL_Delay(2);
    BMI160_CS_High();
    HAL_Delay(5);

    // Perform dummy read to register 0x7F to complete protocol switch
    BMI160_ReadReg(0x7F);
    HAL_Delay(5);

    // 7. Verify Chip ID (Expect 0xD1)
    uint8_t chip_id = BMI160_ReadReg(BMI160_REG_CHIP_ID);
    if (chip_id != BMI160_CHIP_ID_VALUE) {
        return false;
    }

    // 8. Power on Accelerometer (Normal Mode)
    BMI160_WriteReg(BMI160_REG_CMD, BMI160_CMD_ACC_NORMAL);
    HAL_Delay(10); // min 3.8 ms

    // 9. Power on Gyroscope (Normal Mode)
    BMI160_WriteReg(BMI160_REG_CMD, BMI160_CMD_GYR_NORMAL);
    HAL_Delay(60); // min 50 ms

    // 10. Configure Accel (+/- 4g range, 100 Hz ODR)
    BMI160_WriteReg(BMI160_REG_ACC_RANGE, 0x05); // +/-4g
    BMI160_WriteReg(BMI160_REG_ACC_CONF, 0x28);  // Normal filter, 100Hz ODR

    // 11. Configure Gyro (+/- 2000 deg/s range, 100 Hz ODR)
    BMI160_WriteReg(BMI160_REG_GYR_RANGE, 0x00); // +/-2000 dps
    BMI160_WriteReg(BMI160_REG_GYR_CONF, 0x28);  // Normal filter, 100Hz ODR

    return true;
}

uint8_t BMI160_ReadChipID(void) {
    return BMI160_ReadReg(BMI160_REG_CHIP_ID);
}

uint8_t BMI160_GetLastHALStatus(void) {
    return last_hal_status;
}

bool BMI160_ReadRaw(BMI160_RawData_t *accel, BMI160_RawData_t *gyro) {
    uint8_t raw[12] = {0};

    BMI160_ReadMulti(BMI160_REG_DATA_GYRO_X, raw, 12);

    if (gyro) {
        gyro->x = (int16_t)((raw[1] << 8) | raw[0]);
        gyro->y = (int16_t)((raw[3] << 8) | raw[2]);
        gyro->z = (int16_t)((raw[5] << 8) | raw[4]);
    }

    if (accel) {
        accel->x = (int16_t)((raw[7] << 8) | raw[6]);
        accel->y = (int16_t)((raw[9] << 8) | raw[8]);
        accel->z = (int16_t)((raw[11] << 8) | raw[10]);
    }

    return true;
}
