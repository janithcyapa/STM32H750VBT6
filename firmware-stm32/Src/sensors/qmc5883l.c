#include "qmc5883l.h"
#include "i2c_bus.h"
#include <math.h>

#define QMC5883L_REG_DATA_X_LSB 0x00
#define QMC5883L_REG_STATUS     0x06
#define QMC5883L_REG_CTRL1      0x09
#define QMC5883L_REG_CTRL2      0x0A
#define QMC5883L_REG_FBR        0x0B
#define QMC5883L_REG_CHIP_ID    0x0D

uint8_t QMC5883L_ReadChipID(void) {
    uint8_t id = 0;
    I2C2_ReadReg(QMC5883L_I2C_ADDR, QMC5883L_REG_CHIP_ID, &id);
    return id;
}

bool QMC5883L_Init(void) {
    // 1. Soft Reset
    I2C2_WriteReg(QMC5883L_I2C_ADDR, QMC5883L_REG_CTRL2, 0x80);
    HAL_Delay(10);

    // 2. Set SET/RESET period register (0x01 recommended by datasheet)
    I2C2_WriteReg(QMC5883L_I2C_ADDR, QMC5883L_REG_FBR, 0x01);

    // 3. Configure Control Register 1:
    // OSR = 512 (00), Range = 8 Gauss (01), ODR = 50 Hz (01), Mode = Continuous (01)
    // 00 01 01 01 = 0x1D
    I2C2_WriteReg(QMC5883L_I2C_ADDR, QMC5883L_REG_CTRL1, 0x1D);
    HAL_Delay(10);

    // Verify chip responds
    uint8_t id = QMC5883L_ReadChipID();
    // QMC5883L chip ID is 0xFF. Some clones return 0x00 or acknowledge registers.
    // If we can write & read back CTRL1, the sensor is working properly!
    uint8_t ctrl1 = 0;
    I2C2_ReadReg(QMC5883L_I2C_ADDR, QMC5883L_REG_CTRL1, &ctrl1);
    if (ctrl1 == 0x1D || id == QMC5883L_CHIP_ID_VALUE) {
        return true;
    }

    return false;
}

bool QMC5883L_Read(QMC5883L_Data_t *data) {
    uint8_t raw[6] = {0};

    // Burst read 6 data registers (X_LSB, X_MSB, Y_LSB, Y_MSB, Z_LSB, Z_MSB)
    if (!I2C2_ReadMulti(QMC5883L_I2C_ADDR, QMC5883L_REG_DATA_X_LSB, raw, 6)) {
        return false;
    }

    int16_t x = (int16_t)((raw[1] << 8) | raw[0]);
    int16_t y = (int16_t)((raw[3] << 8) | raw[2]);
    int16_t z = (int16_t)((raw[5] << 8) | raw[4]);

    if (data) {
        data->x = x;
        data->y = y;
        data->z = z;

        // Calculate magnetic heading in degrees (0.0 to 360.0)
        float heading = atan2f((float)y, (float)x) * (180.0f / 3.1415926535f);
        if (heading < 0.0f) {
            heading += 360.0f;
        }
        data->heading_deg = heading;
    }

    return true;
}

