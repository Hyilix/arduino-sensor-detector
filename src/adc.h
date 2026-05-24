#ifndef ADC_H
#define ADC_H

#include <avr/io.h>
#include <util/delay.h>

#include <stdint.h>

#include "utils.h"

#define GAS_ANALOG_PIN (uint8_t)0

void adc_init();
void toggle_adc_conversion(uint8_t toggle);
uint16_t adc_read_channel(uint8_t channel);

#endif

