/* AHT20 temperature and humidity */
#ifndef AHT_H
#define AHT_H

#include <avr/io.h>
#include <util/delay.h>

#include <stdint.h>

#include "i2c.h"
#include "uart.h"

#define AHT20_ADDRESS (uint8_t)0x38

typedef struct {
    float temp;
    float hum;
} aht20_data_t;

void aht20_init();
uint8_t aht20_read(aht20_data_t *out);

float aht20_read_temp();
float aht20_read_hum();

#endif

