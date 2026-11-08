#ifndef BMP280_H
#define BMP280_H

#include <stdbool.h>
#include <stdint.h>

#define BMP280_I2C_ADDR_PRIMARY   0x76
#define BMP280_I2C_ADDR_SECONDARY 0x77
#define BMP280_CHIP_ID_VALUE      0x58

typedef struct {
    float temperature_c;  // Temperature in degrees Celsius
    float pressure_hpa;   // Pressure in hectopascals (hPa / mbar)
    float altitude_m;     // Barometric altitude in meters
} BMP280_Data_t;

bool BMP280_Init(void);
uint8_t BMP280_ReadChipID(void);
bool BMP280_Read(BMP280_Data_t *data);

#endif // BMP280_H

