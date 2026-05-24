#ifndef UART_H
#define UART_H

#include <avr/io.h>
#include <util/delay.h>

#include <string.h>
#include <stdint.h>

#include "utils.h"

void uart_init();
void uart_send(char c);
void uart_print(const char *s);
void uart_print_uint16(uint16_t val);
void uart_print_float(float val);

void uart_cursor_home();
void uart_clear_screen();

#endif

