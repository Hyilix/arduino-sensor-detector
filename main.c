#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "utils.h"
#include "i2c.h"
#include "adc.h"
#include "uart.h"
#include "bmp.h"

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

        // Print the temperature from the bmp280
        float temp = bmp280_read_temp();
        uart_print("BMP280 temperature val: ");
        uart_print_float(temp);
        uart_print(" C\r\n");

        // Print the pressure from the bmp280
        float pressure = bmp280_read_pressure();
        uart_print("BMP280 pressure val: ");
        uart_print_float(pressure);
        uart_print(" P\r\n");

        /* ^^^ BMP280 Address based handler ^^^ */

        _delay_ms(LOOP_SLEEP);
    }
}

