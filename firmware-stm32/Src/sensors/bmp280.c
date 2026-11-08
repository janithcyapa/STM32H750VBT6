#include "bmp280.h"
#include "i2c_bus.h"
#include <math.h>

#define BMP280_REG_CHIP_ID   0xD0
#define BMP280_REG_RESET     0xE0
#define BMP280_REG_STATUS    0xF3
#define BMP280_REG_CTRL_MEAS 0xF4
#define BMP280_REG_CONFIG    0xF5
#define BMP280_REG_PRESS_MSB 0xF7
#define BMP280_REG_CALIB     0x88

static uint8_t active_addr = BMP280_I2C_ADDR_PRIMARY;
static int32_t t_fine = 0;

// Calibration parameters
static uint16_t dig_T1;
static int16_t  dig_T2;
static int16_t  dig_T3;
static uint16_t dig_P1;
static int16_t  dig_P2;
static int16_t  dig_P3;
static int16_t  dig_P4;
static int16_t  dig_P5;
static int16_t  dig_P6;
static int16_t  dig_P7;
static int16_t  dig_P8;
static int16_t  dig_P9;

static bool BMP280_ReadCalibration(void) {
    uint8_t calib[24] = {0};
    if (!I2C2_ReadMulti(active_addr, BMP280_REG_CALIB, calib, 24)) {
        return false;
    }

    dig_T1 = (uint16_t)((calib[1] << 8) | calib[0]);
    dig_T2 = (int16_t)((calib[3] << 8) | calib[2]);
    dig_T3 = (int16_t)((calib[5] << 8) | calib[4]);

    dig_P1 = (uint16_t)((calib[7] << 8) | calib[6]);
    dig_P2 = (int16_t)((calib[9] << 8) | calib[8]);
    dig_P3 = (int16_t)((calib[11] << 8) | calib[10]);
    dig_P4 = (int16_t)((calib[13] << 8) | calib[12]);
    dig_P5 = (int16_t)((calib[15] << 8) | calib[14]);
    dig_P6 = (int16_t)((calib[17] << 8) | calib[16]);
    dig_P7 = (int16_t)((calib[19] << 8) | calib[18]);
    dig_P8 = (int16_t)((calib[21] << 8) | calib[20]);
    dig_P9 = (int16_t)((calib[23] << 8) | calib[22]);

    return true;
}

static inline bool is_valid_bmp280_id(uint8_t id) {
    return (id == 0x58 || id == 0x60 || id == 0x56 || id == 0x57);
}

uint8_t BMP280_ReadChipID(void) {
    uint8_t id = 0;
    // Try primary 0x76 first, then secondary 0x77
    if (I2C2_ReadReg(BMP280_I2C_ADDR_PRIMARY, BMP280_REG_CHIP_ID, &id) && is_valid_bmp280_id(id)) {
        active_addr = BMP280_I2C_ADDR_PRIMARY;
        return id;
    }
    if (I2C2_ReadReg(BMP280_I2C_ADDR_SECONDARY, BMP280_REG_CHIP_ID, &id) && is_valid_bmp280_id(id)) {
        active_addr = BMP280_I2C_ADDR_SECONDARY;
        return id;
    }
    return id;
}

bool BMP280_Init(void) {
    uint8_t id = BMP280_ReadChipID();
    if (!is_valid_bmp280_id(id)) {
        return false;
    }

    // Read factory trimming parameters
    if (!BMP280_ReadCalibration()) {
        return false;
    }

    // Configure Standby time = 62.5ms, IIR Filter = 4
    // t_sb = 001 (62.5ms), filter = 010 (coeff 4), spi3w_en = 0 -> 0x28
    I2C2_WriteReg(active_addr, BMP280_REG_CONFIG, 0x28);

    // Configure ctrl_meas:
    // osrs_t = x2 (010), osrs_p = x16 (101), mode = Normal (11) -> 0x57
    I2C2_WriteReg(active_addr, BMP280_REG_CTRL_MEAS, 0x57);

    return true;
}

// Bosch 32-bit compensation formula for Temperature
static int32_t BMP280_Compensate_T(int32_t adc_T) {
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) *
            ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

// Bosch 64-bit compensation formula for Pressure
static uint32_t BMP280_Compensate_P(int32_t adc_P) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) {
        return 0; // avoid divide by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (uint32_t)p;
}

bool BMP280_Read(BMP280_Data_t *data) {
    uint8_t raw[6] = {0};
    if (!I2C2_ReadMulti(active_addr, BMP280_REG_PRESS_MSB, raw, 6)) {
        return false;
    }

    int32_t adc_P = (int32_t)((((uint32_t)raw[0]) << 12) | (((uint32_t)raw[1]) << 4) | (((uint32_t)raw[2]) >> 4));
    int32_t adc_T = (int32_t)((((uint32_t)raw[3]) << 12) | (((uint32_t)raw[4]) << 4) | (((uint32_t)raw[5]) >> 4));

    int32_t temp_raw = BMP280_Compensate_T(adc_T);
    uint32_t press_raw = BMP280_Compensate_P(adc_P);

    if (data) {
        data->temperature_c = (float)temp_raw / 100.0f;
        data->pressure_hpa = ((float)press_raw / 256.0f) / 100.0f; // Pa to hPa

        // Altitude in meters relative to standard sea-level pressure (1013.25 hPa)
        data->altitude_m = 44330.0f * (1.0f - powf(data->pressure_hpa / 1013.25f, 0.1903f));
    }

    return true;
}

