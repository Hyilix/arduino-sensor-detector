#include "aht.h"
#include "i2c.h"

// Send measurement trigger
void aht20_trigger() {
    _delay_ms(15);

    i2c_start();
    i2c_write(AHT20_ADDRESS << 1 | 0);
    i2c_write(0xAC);
    i2c_write(0x33);
    i2c_write(0x00);
    i2c_stop();

    // Wait 80ms after trigger
    _delay_ms(100);
}

// Read 6 bytes of raw data
// Bit[0] will be CRC. While it is read, it is unused
uint8_t aht20_read_raw(uint8_t *buf) {
    aht20_trigger();

    // Retry 5 times
    for (uint8_t ret = 0; ret < 5; ret++) {
        i2c_start();
        i2c_write(AHT20_ADDRESS << 1 | 1);
        for (uint8_t i = 0; i < 7; i++) {
            if (i == 6) {
                buf[i] = i2c_read_nack();
                break;
            }
            buf[i] = i2c_read_ack();
        }
        i2c_stop();

        // If bit[7] == 0, measurement done
        // If bit[7] == 1, continue to wait
        if (!(buf[0] & (1 << 7)))
            return 0;
    }
    return 1;
}

// Read both temperature and humidity at the same time
uint8_t aht20_read(aht20_data_t *out) {
    uint8_t buf[7];

    // Not done
    if (aht20_read_raw(buf))
        return 1;

    uint32_t raw_hum  = (uint32_t)buf[1] << 12
                      | (uint32_t)buf[2] << 4
                      | (uint32_t)buf[3] >> 4;

    uint32_t raw_temp = (uint32_t)(buf[3] & 0x0F) << 16
                      | (uint32_t)buf[4] << 8
                      | (uint32_t)buf[5];

    float temp = (raw_temp / (float)(1UL << 20)) * 200.0f - 50.0f;
    float hum  = (raw_hum  / (float)(1UL << 20)) * 100.0f;

    out->temp = temp;
    out->hum  = hum;
    return 0;
}

void aht20_init() {
    // Wait 40ms after power-on
    _delay_ms(100);

    // Read status
    i2c_start();
    i2c_write(AHT20_ADDRESS << 1 | 0);
    i2c_write(0x71);
    i2c_stop();

    i2c_start();
    i2c_write(AHT20_ADDRESS << 1 | 1);
    uint8_t status = i2c_read_ack();
    i2c_stop();

    if (!(status & (1 << 3))) {
        i2c_start();
        i2c_write(AHT20_ADDRESS << 1 | 0);
        i2c_write(0xBE);
        i2c_write(0x08);
        i2c_write(0x00);
        i2c_stop();

        // Wait 10ms after init
        _delay_ms(10);
    }
}

float aht20_read_temp() {
    uint8_t buf[6];
    // Check if measurement is done while reading raw data
    if (aht20_read_raw(buf)) {
        return -1;
    }

    // Get raw data form of temperature
    uint32_t raw = (uint32_t)(buf[3] & 0x0F) << 16
                 | (uint32_t)buf[4] << 8
                 | (uint32_t)buf[5];

    // Temperature formula from raw
    return (raw / (float)(1UL << 20)) * 200.0f - 50.0f;
}

float aht20_read_hum() {
    uint8_t buf[6];
    // Check if measurement is done while reading raw data
    if (aht20_read_raw(buf)) {
        return -1;
    }

    // Get raw data form of humidity
    uint32_t raw = (uint32_t)buf[1] << 12
                 | (uint32_t)buf[2] << 4
                 | (uint32_t)buf[3] >> 4;

    // Humidity formula from raw
    return (raw / (float)(1UL << 20)) * 100.0f;
}

