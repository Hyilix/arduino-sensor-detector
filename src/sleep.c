#include "sleep.h"

// Enable watchdog function
void watchdog_enable() {
    // Disable interrupts
    cli();

    // Reset watchdog timer
    wdt_reset();

    // Interrupt, then go to System Reset Mode
    WDTCSR |= (1 << WDCE) | (1 << WDE);

    // Typical time-out: 250ms at 5V
    WDTCSR = (1 << WDIE) | (1 << WDP2);

    // Re-enable interrupts
    sei();
}

// Method to sleep for parts of 250ms
void sleep_250ms(uint8_t parts) {
    for (uint8_t i = 0; i < parts; i++) {
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();

        // Disable peripherals before sleep
        power_adc_disable();
        power_spi_disable();
        power_usart0_disable();
        power_twi_disable();

        // Wakes after watchdog fires
        sleep_cpu();

        // Re-enable peripherals after wake
        sleep_disable();
        power_adc_enable();
        power_usart0_enable();
        power_twi_enable();

        // Let everything stabilize
        _delay_ms(5);
    }
}

// Wrapper to sleep for seconds
void sleep_seconds(uint8_t seconds) {
    for (uint8_t i = 0; i < seconds; i++) {
        sleep_250ms(4);
    }
}


