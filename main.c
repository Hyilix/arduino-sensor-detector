#include "utils.h"
#include "i2c.h"
#include "adc.h"
#include "uart.h"
#include "bmp.h"
#include "aht.h"

int main() {
    // Initialisation
    i2c_init();
    adc_init();
    uart_init();
    aht20_init();

    bmp280_read_calib();

    // Main loop
    while (1) {
        uart_cursor_home();

        uart_print("Sensors readings -- Refreshing every ");
        uart_print_uint16((uint16_t)LOOP_SLEEP);
        uart_print("ms\r\n");

        /* vvv FC-22 Analog handler vvv */
        uint16_t gas_adc_val = adc_read_channel(GAS_ANALOG_PIN);

        uart_print("FC-22 analog GAS value: ");
        uart_print_uint16(gas_adc_val);
        uart_print("\r\n");

        uart_print("\r\n");

        /* ^^^ FC-22 Analog handler ^^^ */

        /* vvv BMP280 Address based handler vvv */

        /* Uncomment this for printing id of the device */
        // uint8_t bmp_id;
        // uart_print("BMP280 ID val: ");
        // i2c_read_register(BMP280_ADDR, 0xD0, &bmp_id, 1);
        // uart_print_uint16(bmp_id);
        // uart_print("\r\n");

        // Print the temperature from the bmp280
        float bmp_temp = bmp280_read_temp();
        uart_print("BMP280 temperature val: ");
        uart_print_float(bmp_temp);
        uart_print(" C\r\n");

        // Print the pressure from the bmp280
        float bmp_pressure = bmp280_read_pressure();
        uart_print("BMP280 pressure val: ");
        uart_print_float(bmp_pressure);
        uart_print(" P\r\n");

        uart_print("\r\n");

        /* ^^^ BMP280 Address based handler ^^^ */

        /* vvv AHT20 handler vvv */

        // Print the temperature from the aht20
        float aht_temp = aht20_read_temp();
        uart_print("AHT20 temperature val: ");
        uart_print_float(aht_temp);
        uart_print(" C\r\n");

        // Print the humidity from the aht20
        float aht_hum = aht20_read_hum();
        uart_print("AHT20 humidity val: ");
        uart_print_float(aht_hum);
        uart_print(" %RH\r\n");

        /* ^^^ AHT20 handler ^^^ */

        _delay_ms(LOOP_SLEEP);
    }
}

