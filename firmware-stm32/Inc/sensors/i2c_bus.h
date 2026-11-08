#ifndef I2C_BUS_H
#define I2C_BUS_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

// Primary Sensor I2C Bus on PB8 (SCL) and PB9 (SDA) -> I2C1
extern I2C_HandleTypeDef hi2c1;

bool I2C_Bus_Init(void);
uint8_t I2C_Bus_Scan(char *out_buf, uint16_t max_len);
bool I2C_ReadReg(uint8_t dev_addr, uint8_t reg, uint8_t *val);
bool I2C_WriteReg(uint8_t dev_addr, uint8_t reg, uint8_t val);
bool I2C_ReadMulti(uint8_t dev_addr, uint8_t start_reg, uint8_t *buf, uint16_t len);

// Compatibility aliases
#define hi2c2            hi2c1
#define I2C2_Bus_Init    I2C_Bus_Init
#define I2C2_Bus_Scan    I2C_Bus_Scan
#define I2C2_ReadReg     I2C_ReadReg
#define I2C2_WriteReg    I2C_WriteReg
#define I2C2_ReadMulti   I2C_ReadMulti

#endif // I2C_BUS_H

