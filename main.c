#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>

#define LOOP_SLEEP 500

#define F_CPU 16000000UL
#define SCL_FREQ 50000UL
#define BAUD 9600
#define UBRR_VAL (F_CPU/16/BAUD - 1)

#define GAS_ANALOG_PIN (uint8_t)0

#define BMP280_ADDR (uint8_t)0x77

// I2C control
/* vvv I2C functions vvv */

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

/* ^^^ I2C functions ^^^ */

// Analog to Digital Converter
/* vvv ADC functions vvv */

// Init adc
void adc_init() {
    ADMUX = (1 << REFS0);

    // Enable ADSRA. For maximum resolution, 50kHz-200kHz is required
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

// Toggle adc conversion. 0 -> stop | 1 -> start
void toggle_adc_conversion(uint8_t toggle) {
    if (!toggle) {
        ADCSRA |= (1 << ADSC);
        return;
    }
    ADCSRA &= ~(1 << ADSC);
}

// Read data from analog channel
uint16_t adc_read_channel(uint8_t channel) {
    // Read channel into ADMUX
    // Keep the info of the first 4 bits, then select the MUX0-3 on the last 4 with the channel
    ADMUX = (ADMUX & 0xF0) | channel;

    // Start ADC conversion
    toggle_adc_conversion(0);

    // Read ADSC
    while (ADCSRA & (1 << ADSC));

    // Stop ADC conversion
    toggle_adc_conversion(1);

    return ADC;
}

/* ^^^ ADC functions ^^^ */

/* vvv UART funcitons vvv */

// Init uart
void uart_init() {
    // Baud rate
    UBRR0H = (UBRR_VAL >> 8);
    UBRR0L = UBRR_VAL;

    // Enable TX
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Send a byte of data
void uart_send(char c) {
    // Wait for buffer
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

// Print char by char
void uart_print(const char *s) {
    while (*s) uart_send(*s++);
}

// Print uint16_t via uart
void uart_print_uint16(uint16_t val) {
    char buf[6];
    memset(buf, '0', 5);
    buf[5] = '\0';

    // Send 0 via uart
    if (val == 0) {
        uart_send('0');
        return;
    }

    // Convert uint16 to string
    for (int i = 4; i >= 0 && val > 0; i--) {
        buf[i] = '0' + val % 10;
        val /= 10;
    }

    int start = 0;
    while (start < 5 && buf[start] == '0') {
        start++;
    }
    uart_print(buf + start);
}

/* ^^^ UART functions ^^^ */

int main() {
    // Initialisation
    i2c_init();
    adc_init();
    uart_init();

    // uart_print("Scanning...\r\n");
    // _delay_ms(1000);
    //
    // for (uint8_t addr = 1; addr < 127; addr++) {
    //     i2c_start();
    //     i2c_write(addr << 1 | 0);
    //     uint8_t status = TWSR & 0xF8;
    //     i2c_stop();
    //
    //     if (status == 0x18) {  // ACK received = device found
    //         uart_print("Found: 0x");
    //         uart_print_uint16(addr);
    //         uart_print("\r\n");
    //     }
    // }
    //
    // uart_print("Done\r\n");

    // Main loop
    while (1) {
        // ADC handler
        uint16_t gas_adc_val = adc_read_channel(GAS_ANALOG_PIN);

        uart_print("Analog GAS value: ");
        uart_print_uint16(gas_adc_val);
        uart_print("\r\n");

        // BMP280 Address based hadnelr
        uart_print("BMP280 ID val: ");

        uint8_t buf;
        i2c_read_register(BMP280_ADDR, 0xD0, &buf, 1);
        uart_print_uint16(buf);
        uart_print("\r\n");

        _delay_ms(LOOP_SLEEP);
    }
}
