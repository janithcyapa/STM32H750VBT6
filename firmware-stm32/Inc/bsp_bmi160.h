#ifndef BSP_BMI160_H
#define BSP_BMI160_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

// BMI160 SPI Registers
#define BMI160_REG_CHIP_ID      0x00
#define BMI160_REG_DATA_GYRO_X  0x0C
#define BMI160_REG_DATA_ACC_X   0x12
#define BMI160_REG_ACC_CONF     0x40
#define BMI160_REG_ACC_RANGE    0x41
#define BMI160_REG_GYR_CONF     0x42
#define BMI160_REG_GYR_RANGE    0x43
#define BMI160_REG_CMD          0x7E

// Expected Chip ID for BMI160
#define BMI160_CHIP_ID_VALUE    0xD1

// Commands
#define BMI160_CMD_ACC_NORMAL   0x11
#define BMI160_CMD_GYR_NORMAL   0x15
#define BMI160_CMD_SOFT_RESET   0xB6

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} BMI160_RawData_t;

// API
bool BSP_BMI160_Init(void);
uint8_t BSP_BMI160_ReadChipID(void);
bool BSP_BMI160_ReadRaw(BMI160_RawData_t *accel, BMI160_RawData_t *gyro);

#endif // BSP_BMI160_H

