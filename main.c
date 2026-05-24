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

typedef struct {
    uint16_t T1;
    int16_t  T2;
    int16_t  T3;
    uint16_t P1;
    int16_t  P2;
    int16_t  P3;
    int16_t  P4;
    int16_t  P5;
    int16_t  P6;
    int16_t  P7;
    int16_t  P8;
    int16_t  P9;
} bmp280_calib_t;

// T_fine from bmp280_read_temp
int32_t t_fine;

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

// Print float via uart
void uart_print_float(float val) {
    uint16_t whole   = (uint16_t)val;
    uint16_t decimal = (uint16_t)(val * 100) % 100;

    uart_print_uint16(whole);
    uart_send('.');
    uart_print_uint16(decimal);
}

/* ^^^ UART functions ^^^ */

/* vvv BMP functions vvv */
bmp280_calib_t calib;

// BMP read calibration for temperature and pressure
void bmp280_read_calib() {
    uint8_t buf[24];
    i2c_read_register(BMP280_ADDR, 0x88, buf, 24);

    calib.T1 = (uint16_t)(buf[1] << 8) | buf[0];
    calib.T2 = (int16_t) (buf[3] << 8) | buf[2];
    calib.T3 = (int16_t) (buf[5] << 8) | buf[4];

    calib.P1 = (uint16_t)(buf[7]  << 8) | buf[6];
    calib.P2 = (int16_t) (buf[9]  << 8) | buf[8];
    calib.P3 = (int16_t) (buf[11] << 8) | buf[10];
    calib.P4 = (int16_t) (buf[13] << 8) | buf[12];
    calib.P5 = (int16_t) (buf[15] << 8) | buf[14];
    calib.P6 = (int16_t) (buf[17] << 8) | buf[16];
    calib.P7 = (int16_t) (buf[19] << 8) | buf[18];
    calib.P8 = (int16_t) (buf[21] << 8) | buf[20];
    calib.P9 = (int16_t) (buf[23] << 8) | buf[22];
}

float bmp280_read_temp() {
    // Supersample x1 temp and pressure, forced
    uint8_t ctrl = (1 << 5) | (1 << 2) | 1;
    i2c_write_register(BMP280_ADDR, 0xF4, ctrl);
    _delay_ms(10);

    // Read temperature
    uint8_t buf[3];
    i2c_read_register(BMP280_ADDR, 0xFA, buf, 3);

    int32_t raw = (int32_t)buf[0] << 12
                | (int32_t)buf[1] << 4
                | (int32_t)buf[2] >> 4;

    // Compensation formula (Source: Bosch)
    int32_t var1 = ((((raw >> 3) - ((int32_t)calib.T1 << 1)))
                  * ((int32_t)calib.T2)) >> 11;
    int32_t var2 = (((((raw >> 4) - ((int32_t)calib.T1))
                  * ((raw >> 4) - ((int32_t)calib.T1))) >> 12)
                  * ((int32_t)calib.T3)) >> 14;

    t_fine = var1 + var2;
    return (float)((t_fine * 5 + 128) >> 8) / 100.0f;
}

float bmp280_read_pressure() {
    uint8_t buf[3];
    i2c_read_register(BMP280_ADDR, 0xF7, buf, 3);

    int32_t raw_press = ((int32_t)buf[0] << 12)
                      | ((int32_t)buf[1] << 4)
                      | (buf[2] >> 4);

    // Bosch compensation formula — needs t_fine from temp calculation
    int64_t var1 = ((int64_t)t_fine) - 128000;
    int64_t var2 = var1 * var1 * (int64_t)calib.P6;
    var2 = var2 + ((var1 * (int64_t)calib.P5) << 17);
    var2 = var2 + (((int64_t)calib.P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.P3) >> 8)
         + ((var1 * (int64_t)calib.P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib.P1) >> 33;

    if (var1 == 0) return 0;  // avoid division by zero

    int64_t p = 1048576 - raw_press;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib.P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib.P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.P7) << 4);

    return (float)p / 256.0f;  // result in Pascals
}

/* ^^^ BMP functions ^^^ */

int main() {
    // Initialisation
    i2c_init();
    adc_init();
    uart_init();

    bmp280_read_calib();

    // Main loop
    while (1) {
        /* vvv ADC handler vvv */
        uint16_t gas_adc_val = adc_read_channel(GAS_ANALOG_PIN);

        uart_print("Analog GAS value: ");
        uart_print_uint16(gas_adc_val);
        uart_print("\r\n");

        /* ^^^ ADC handler ^^^ */

        /* vvv BMP280 Address based handler vvv */

        /* Uncomment this for printing id of the device */
        // uint8_t bmp_id;
        // uart_print("BMP280 ID val: ");
        // i2c_read_register(BMP280_ADDR, 0xD0, &bmp_id, 1);
        // uart_print_uint16(bmp_id);
        // uart_print("\r\n");

        // Print the temperature for the bmp280
        float temp = bmp280_read_temp();
        uart_print("BMP280 temperature val: ");
        uart_print_float(temp);
        uart_print(" C\r\n");

        float pressure = bmp280_read_pressure();
        uart_print("BMP280 pressure val: ");
        uart_print_float(pressure);
        uart_print(" P\r\n");

        /* ^^^ BMP280 Address based handler ^^^ */

        _delay_ms(LOOP_SLEEP);
    }
}
