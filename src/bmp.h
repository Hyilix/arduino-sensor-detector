/* BMP280 temperature and pressure senzor */
#ifndef BMP_H
#define BMP_H

#include <avr/io.h>
#include <util/delay.h>

#include <stdint.h>

#include "i2c.h"

#define BMP280_ADDR (uint8_t)0x77

typedef struct {
    uint16_t T1;
    int16_t  T2;
    int16_t  T3;
    uint16_t P1;
    int16_t  P2;
    int16_t  P3;
    int16_t  P4;
    int16_t  P5;
    int16_t  P6;
    int16_t  P7;
    int16_t  P8;
    int16_t  P9;
} bmp280_calib_t;

void bmp280_read_calib();
float bmp280_read_temp();
float bmp280_read_pressure();

#endif

