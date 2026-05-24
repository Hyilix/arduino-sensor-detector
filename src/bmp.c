#include "bmp.h"

// T_fine from bmp280_read_temp
int32_t t_fine;

bmp280_calib_t calib;

// BMP read calibration for temperature and pressure
void bmp280_read_calib() {
    uint8_t buf[24];
    i2c_read_register(BMP280_ADDR, 0x88, buf, 24);

    calib.T1 = (uint16_t)(buf[1] << 8) | buf[0];
    calib.T2 = (int16_t) (buf[3] << 8) | buf[2];
    calib.T3 = (int16_t) (buf[5] << 8) | buf[4];

    calib.P1 = (uint16_t)(buf[7]  << 8) | buf[6];
    calib.P2 = (int16_t) (buf[9]  << 8) | buf[8];
    calib.P3 = (int16_t) (buf[11] << 8) | buf[10];
    calib.P4 = (int16_t) (buf[13] << 8) | buf[12];
    calib.P5 = (int16_t) (buf[15] << 8) | buf[14];
    calib.P6 = (int16_t) (buf[17] << 8) | buf[16];
    calib.P7 = (int16_t) (buf[19] << 8) | buf[18];
    calib.P8 = (int16_t) (buf[21] << 8) | buf[20];
    calib.P9 = (int16_t) (buf[23] << 8) | buf[22];
}

float bmp280_read_temp() {
    // Supersample x1 temp and pressure, forced
    uint8_t ctrl = (1 << 5) | (1 << 2) | 1;
    i2c_write_register(BMP280_ADDR, 0xF4, ctrl);
    _delay_ms(10);

    // Read temperature
    uint8_t buf[3];
    i2c_read_register(BMP280_ADDR, 0xFA, buf, 3);

    int32_t raw = (int32_t)buf[0] << 12
                | (int32_t)buf[1] << 4
                | (int32_t)buf[2] >> 4;

    // Compensation formula (Source: Bosch)
    int32_t var1 = ((((raw >> 3) - ((int32_t)calib.T1 << 1)))
                  * ((int32_t)calib.T2)) >> 11;
    int32_t var2 = (((((raw >> 4) - ((int32_t)calib.T1))
                  * ((raw >> 4) - ((int32_t)calib.T1))) >> 12)
                  * ((int32_t)calib.T3)) >> 14;

    t_fine = var1 + var2;
    return (float)((t_fine * 5 + 128) >> 8) / 100.0f;
}

float bmp280_read_pressure() {
    // Read pressure
    uint8_t buf[3];
    i2c_read_register(BMP280_ADDR, 0xF7, buf, 3);

    int32_t raw_press = ((int32_t)buf[0] << 12)
                      | ((int32_t)buf[1] << 4)
                      | (buf[2] >> 4);

    // Compensation formula (Source: Bosch)
    int64_t var1 = ((int64_t)t_fine) - 128000;
    int64_t var2 = var1 * var1 * (int64_t)calib.P6;
    var2 = var2 + ((var1 * (int64_t)calib.P5) << 17);
    var2 = var2 + (((int64_t)calib.P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.P3) >> 8)
         + ((var1 * (int64_t)calib.P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib.P1) >> 33;

    if (var1 == 0) return 0;  // avoid division by zero

    int64_t p = 1048576 - raw_press;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib.P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib.P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.P7) << 4);

    return (float)p / 256.0f;
}

