#include "utils.h"
#include "i2c.h"
#include "adc.h"
#include "uart.h"
#include "bmp.h"
#include "aht.h"

aht20_data_t aht_data;

int main() {
    // Initialisation
    i2c_init();
    adc_init();
    uart_init();
    aht20_init();

    bmp280_read_calib();

    // Initialise min-max values
    uint16_t gas_min = (uint16_t)(1 << 15);
    uint16_t gas_max = 0;

    float bmp_min_temp = (float)(1UL << 31);
    float bmp_max_temp = 0;

    float bmp_min_prs = (float)(1UL << 31);
    float bmp_max_prs = 0;

    float aht_min_temp = (float)(1UL << 31);
    float aht_max_temp = 0;

    float aht_min_hum = (float)(1UL << 31);
    float aht_max_hum = 0;


    // uart_print("HELLO\n");

    // Main loop
    while (1) {
        uart_cursor_home();

        uart_print("Sensors readings -- Refreshing every ");
        uart_print_uint16((uint16_t)LOOP_SLEEP);
        uart_print("ms\r\n");

        uart_print("\r\n");

        /* vvv FC-22 Analog handler vvv */
        uart_print("FC-22 Analog (ADC) Readings:\r\n");

        uint16_t gas_adc_val = adc_read_channel(GAS_ANALOG_PIN);

        if (gas_adc_val < gas_min) {
            gas_min = gas_adc_val;
        }

        if (gas_adc_val > gas_max) {
            gas_max = gas_adc_val;
        }

        uart_print("\tGAS current: ");
        uart_print_uint16(gas_adc_val);
        uart_print("\r\n");

        uart_print("\tGAS min:     ");
        uart_print_uint16(gas_min);
        uart_print("\r\n");

        uart_print("\tGAS max:     ");
        uart_print_uint16(gas_max);
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

        uart_print("BMP280 Readings:\r\n");

        // Print the temperature from the bmp280
        float bmp_temp = bmp280_read_temp();

        if (bmp_temp < bmp_min_temp) {
            bmp_min_temp = bmp_temp;
        }

        if (bmp_temp > bmp_max_temp) {
            bmp_max_temp = bmp_temp;
        }

        uart_print("\tTemperature current: ");
        uart_print_float(bmp_temp);
        uart_print(" C");
        uart_clear_line();
        uart_print("\r\n");

        uart_print("\tTemperature min:     ");
        uart_print_float(bmp_min_temp);
        uart_print(" C");
        uart_clear_line();
        uart_print("\r\n");

        uart_print("\tTemperature max:     ");
        uart_print_float(bmp_max_temp);
        uart_print(" C");
        uart_clear_line();
        uart_print("\r\n");

        uart_print("\r\n");

        // Print the pressure from the bmp280
        float bmp_pressure = bmp280_read_pressure();

        if (bmp_pressure < bmp_min_prs) {
            bmp_min_prs = bmp_pressure;
        }

        if (bmp_pressure > bmp_max_prs) {
            bmp_max_prs = bmp_pressure;
        }

        uart_print("\tPressure current: ");
        uart_print_float(bmp_pressure / 100.0f);
        uart_print(" hPa");
        uart_clear_line();
        uart_print("\r\n");

        uart_print("\tPressure min:     ");
        uart_print_float(bmp_min_prs / 100.0f);
        uart_print(" hPa");
        uart_clear_line();
        uart_print("\r\n");

        uart_print("\tPressure max:     ");
        uart_print_float(bmp_max_prs / 100.0f);
        uart_print(" hPa");
        uart_clear_line();
        uart_print("\r\n");

        uart_print("\r\n");

        /* ^^^ BMP280 Address based handler ^^^ */

        /* vvv AHT20 handler vvv */

        uart_print("AHT20 Readings:\r\n");

        uint8_t sanity = aht20_read(&aht_data);
        if (!sanity) {
            // Print the temperature from the aht20
            float aht_temp = aht_data.temp;//aht20_read_temp();

            if (aht_temp < aht_min_temp) {
                aht_min_temp = aht_temp;
            }

            if (aht_temp > aht_max_temp) {
                aht_max_temp = aht_temp;
            }

            uart_print("\tTemperature current: ");
            uart_print_float(aht_temp);
            uart_print(" C");
            uart_clear_line();
            uart_print("\r\n");

            uart_print("\tTemperature min:     ");
            uart_print_float(aht_min_temp);
            uart_print(" C");
            uart_clear_line();
            uart_print("\r\n");

            uart_print("\tTemperature max:     ");
            uart_print_float(aht_max_temp);
            uart_print(" C");
            uart_clear_line();
            uart_print("\r\n");

            uart_print("\r\n");

            // Print the humidity from the aht20
            float aht_hum = aht_data.hum;//aht20_read_hum();

            if (aht_hum < aht_min_hum) {
                aht_min_hum = aht_hum;
            }

            if (aht_hum > aht_max_hum) {
                aht_max_hum = aht_hum;
            }

            uart_print("\tHumidity current: ");
            uart_print_float(aht_hum);
            uart_print(" %RH");
            uart_clear_line();
            uart_print("\r\n");

            uart_print("\tHumidity min:     ");
            uart_print_float(aht_min_hum);
            uart_print(" %RH");
            uart_clear_line();
            uart_print("\r\n");

            uart_print("\tHumidity max:     ");
            uart_print_float(aht_max_hum);
            uart_print(" %RH");
            uart_clear_line();
            uart_print("\r\n");
        }
        else {
            uart_print("AHT20 READING FAILED\r\n");
        }

        /* ^^^ AHT20 handler ^^^ */

        _delay_ms(LOOP_SLEEP);
    }
}

