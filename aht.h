/* AHT20 temperature and humidity */
#ifndef AHT_H
#define AHT_H

#include <avr/io.h>
#include <util/delay.h>

#include <stdint.h>

#include "i2c.h"

#define AHT20_ADDRESS (uint8_t)0x38

void aht20_init();
float aht20_read_temp();
float aht20_read_hum();

#endif

