#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>

#define F_CPU 16000000UL
#define BAUD 9600
#define UBRR_VAL (F_CPU/16/BAUD - 1)

#define GAS_ANALOG_PIN (uint8_t)0

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

    UCSR0B = (1 << TXEN0);                 // Enable TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data
}

// Send a byte of data
void uart_send(char c) {
    while (!(UCSR0A & (1 << UDRE0)));   // Wait for buffer
    UDR0 = c;
}

// Print char by char
void uart_print(const char *s) {
    while (*s)
        uart_send(*s++);
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
    adc_init();
    uart_init();
    while (1) {
        uint16_t gas_adc_val = adc_read_channel(GAS_ANALOG_PIN);

        uart_print("Analog GAS value: ");
        uart_print_uint16(gas_adc_val);
        uart_print("\r\n");

        _delay_ms(100);
    }
}
