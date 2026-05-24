#include "adc.h"

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
