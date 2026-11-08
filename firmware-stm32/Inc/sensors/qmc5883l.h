#ifndef QMC5883L_H
#define QMC5883L_H

#include <stdbool.h>
#include <stdint.h>

#define QMC5883L_I2C_ADDR       0x0D
#define QMC5883L_CHIP_ID_VALUE  0xFF

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
    float heading_deg; // Compass azimuth heading (0.0 to 360.0 degrees)
} QMC5883L_Data_t;

bool QMC5883L_Init(void);
uint8_t QMC5883L_ReadChipID(void);
bool QMC5883L_Read(QMC5883L_Data_t *data);

#endif // QMC5883L_H

