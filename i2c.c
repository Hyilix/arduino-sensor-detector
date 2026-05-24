#include "i2c.h"

// Init i2c
void i2c_init() {
    // Prescaler = 1
    TWSR = 0;

    // Set the bit rate register
    TWBR = (uint8_t)((F_CPU / SCL_FREQ - 16) / 2);

    // Enable TWI
    TWCR = (1 << TWEN);
}

// Wait for TWINT flag
void i2c_wait() {
    while (!(TWCR & (1 << TWINT)));
}

// Send START condition
void i2c_start() {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    i2c_wait();
}

// Send STOP condition
void i2c_stop() {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    while (TWCR & (1 << TWSTO));
}

// Read byte, send ACK
uint8_t i2c_read_ack() {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    i2c_wait();
    return TWDR;
}

// Read byte, send NACK
uint8_t i2c_read_nack() {
    TWCR = (1 << TWINT) | (1 << TWEN);
    i2c_wait();
    return TWDR;
}

// Write data
void i2c_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    i2c_wait();
}

// Read from register of a device (addr)
void i2c_read_register(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len) {
    i2c_start();
    i2c_write(addr << 1 | 0);   // Write to addr + write bit
    i2c_write(reg);             // Write to reg
    i2c_stop();

    i2c_start();
    i2c_write(addr << 1 | 1);   // Write to addr + read bit

    for (uint8_t i = 0; i < len; i++) {
        // Done reading
        if (i == len - 1) {
            buf[i] = i2c_read_nack();
            break;
        }
        buf[i] = i2c_read_ack();
    }

    i2c_stop();
}

// Write data to register of a device (addr)
void i2c_write_register(uint8_t addr, uint8_t reg, uint8_t data) {
    i2c_start();
    i2c_write(addr << 1 | 0);
    i2c_write(reg);
    i2c_write(data);
    i2c_stop();
}

