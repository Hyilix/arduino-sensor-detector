#include "uart.h"

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

// Print float via uart
void uart_print_float(float val) {
    uint16_t whole   = (uint16_t)val;
    uint16_t decimal = (uint16_t)(val * 100) % 100;

    uart_print_uint16(whole);
    uart_send('.');
    uart_print_uint16(decimal);
    // Pad the decimal to use 2 digits
    if (decimal < 10) {
        uart_send('0');
    }
}

// Set the cursor at the top via uart
void uart_cursor_home() {
    uart_print("\033[H");
}

// Clear the screen via uart
void uart_clear_screen() {
    uart_print("\033[2J");
    uart_cursor_home();
}

// Erase from cursor to end of line
void uart_clear_line() {
    uart_print("\033[K");
}

