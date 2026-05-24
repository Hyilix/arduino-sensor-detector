#ifndef SLEEP_H
#define SLEEP_H

#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#include <stdint.h>

void watchdog_enable();
void sleep_250ms(uint8_t parts);
void sleep_seconds(uint8_t seconds);

#endif

