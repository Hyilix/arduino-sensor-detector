#ifndef I2C_H
#define I2C_H

#include <avr/io.h>
#include <util/delay.h>

#include <stdint.h>

#include "utils.h"

void i2c_init();
void i2c_wait();
void i2c_start();
void i2c_stop();
uint8_t i2c_read_ack();
uint8_t i2c_read_nack();
void i2c_write(uint8_t data);
void i2c_read_register(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len);
void i2c_write_register(uint8_t addr, uint8_t reg, uint8_t data);

#endif

