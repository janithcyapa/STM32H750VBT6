#include "i2c_bus.h"
#include <stdio.h>
#include <string.h>

I2C_HandleTypeDef hi2c1 = {0};

static void I2C_Bus_Clear(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
    HAL_Delay(1);

    // If SDA (PB9) is held low by a slave, clock SCL (PB8) 9 times
    for (int i = 0; i < 9; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
        for (volatile int d = 0; d < 1000; d++);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
        for (volatile int d = 0; d < 1000; d++);
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == GPIO_PIN_SET) {
            break;
        }
    }

    // Generate software STOP condition (SDA low -> SCL high -> SDA high)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
    for (volatile int d = 0; d < 500; d++);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
    for (volatile int d = 0; d < 500; d++);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
    for (volatile int d = 0; d < 500; d++);
}

bool I2C_Bus_Init(void) {
    // 1. Bus recovery sequence
    I2C_Bus_Clear();

    // 2. Reset I2C1 peripheral to clean state
    __HAL_RCC_I2C1_FORCE_RESET();
    for (volatile int d = 0; d < 1000; d++);
    __HAL_RCC_I2C1_RELEASE_RESET();

    // 3. Configure Kernel Clock for I2C1/2/3 -> HSI (64 MHz)
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C123;
    PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_HSI;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        return false;
    }

    // 4. Enable GPIOB and I2C1 Clocks
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    // 5. Configure PB8 (SCL) and PB9 (SDA) as I2C1 AF4 (Open-Drain + Internal Pull-Up)
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gpio);

    // 6. Configure I2C1 Peripheral (100 kHz Standard Mode)
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10707DBC; // 100 kHz at 64 MHz HSI
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    return (HAL_I2C_Init(&hi2c1) == HAL_OK);
}

uint8_t I2C_Bus_Scan(char *out_buf, uint16_t max_len) {
    uint8_t count = 0;
    int offset = 0;

    // Check raw GPIO pin voltages and peripheral status
    uint8_t scl_val = (GPIOB->IDR & GPIO_PIN_8) ? 1 : 0;
    uint8_t sda_val = (GPIOB->IDR & GPIO_PIN_9) ? 1 : 0;
    uint32_t isr_val = hi2c1.Instance ? hi2c1.Instance->ISR : 0;

    offset += snprintf(out_buf + offset, max_len - offset,
                       "\r\n[I2C1 Scanner] Probing 7-bit addresses (0x01..0x7F)...\r\n"
                       "  Line status: PB8(SCL)=%s, PB9(SDA)=%s | ISR=0x%08lX\r\n",
                       scl_val ? "HIGH" : "LOW",
                       sda_val ? "HIGH" : "LOW",
                       isr_val);

    for (uint8_t addr = 1; addr < 128; addr++) {
        // Reset peripheral if stuck busy
        if (__HAL_I2C_GET_FLAG(&hi2c1, I2C_FLAG_BUSY)) {
            __HAL_RCC_I2C1_FORCE_RESET();
            for (volatile int w = 0; w < 500; w++);
            __HAL_RCC_I2C1_RELEASE_RESET();
            HAL_I2C_Init(&hi2c1);
        }

        // Probe address
        if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr << 1), 1, 5) == HAL_OK) {
            count++;
            const char *label = "Unknown";
            if (addr == 0x0D) label = "QMC5883L Magnetometer (GY-271)";
            else if (addr == 0x1E) label = "HMC5883L Magnetometer";
            else if (addr == 0x76 || addr == 0x77) label = "BMP280 Barometer";
            else if (addr == 0x68 || addr == 0x69) label = "BMI160/MPU (I2C Mode)";

            offset += snprintf(out_buf + offset, max_len - offset,
                               "  -> Found device at 0x%02X (%s)\r\n", addr, label);
            if (offset >= max_len - 64) break;
        }
    }

    if (count == 0) {
        HAL_StatusTypeDef s_mag = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(0x0D << 1), 1, 10);
        HAL_StatusTypeDef s_baro = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(0x76 << 1), 1, 10);
        offset += snprintf(out_buf + offset, max_len - offset,
                           "  -> No ACK! Probe 0x0D status: %u, Probe 0x76 status: %u\r\n"
                           "     (0=OK, 1=ERROR/NACK, 2=BUSY, 3=TIMEOUT)\r\n",
                           s_mag, s_baro);
    } else {
        offset += snprintf(out_buf + offset, max_len - offset,
                           "  -> Scan complete: %u device(s) detected.\r\n", count);
    }

    return count;
}

bool I2C_ReadReg(uint8_t dev_addr, uint8_t reg, uint8_t *val) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(dev_addr << 1),
                                                reg, I2C_MEMADD_SIZE_8BIT,
                                                val, 1, 50);
    return (status == HAL_OK);
}

bool I2C_WriteReg(uint8_t dev_addr, uint8_t reg, uint8_t val) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&hi2c1, (uint16_t)(dev_addr << 1),
                                                 reg, I2C_MEMADD_SIZE_8BIT,
                                                 &val, 1, 50);
    return (status == HAL_OK);
}

bool I2C_ReadMulti(uint8_t dev_addr, uint8_t start_reg, uint8_t *buf, uint16_t len) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(dev_addr << 1),
                                                start_reg, I2C_MEMADD_SIZE_8BIT,
                                                buf, len, 100);
    return (status == HAL_OK);
}
