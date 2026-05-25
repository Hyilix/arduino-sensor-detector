# Arduino Sensor Detector

Reading different sensors on an Arduino board

> Ursescu Sebastian - 325CA

Github repo: https://github.com/Hyilix/arduino-sensor-detector

Video Youtube link: https://www.youtube.com/watch?v=jLT6DfCp0Xs

> Video available as file `Arduino_Sensor_Video.mp4`

Basic specs:
  * MCU: `atmega328p`
  * CLOCK: `16.000.000Hz` -> `16MHz`
  * BAUDRATE: `9600`

Sensors used:
  * FC-22 analog gas (Analog Digital Converted): https://electronicsforyou.com/product/gas-sensor-fc-22-mq-9/
  * BMP280/AHT20 combo:
      * BMP280 (temperature + pressure): https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmp280-ds001.pdf
      * AHT20 (temperature + humidity): https://files.seeedstudio.com/wiki/Grove-AHT20_I2C_Industrial_Grade_Temperature_and_Humidity_Sensor/AHT20-datasheet-2020-4-16.pdf

## Technical Reference

### MCU

ATmega328P — 8-bit AVR RISC microcontroller by Microchip, mounted on Arduino Uno R3.

Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf

Electrical Parameters:
  * Supply voltage: `1.8V – 5.5V`
  * Nominal operating voltage: `5V`
  * Max clock at 5V: `20 MHz`
  * Nominal clock (Arduino): `16 MHz`
  * Running current @ 5V 16MHz: `~12 mA`
  * IDLE mode current @ 5V 16MHz: `~3.5 mA`
  * Power-down mode current: `~0.1 µA`

### Block Diagram

```
  [FC-22]  ──── ADC (A0) ────┐
  [BMP280] ──── I2C (A4/A5) ─┤─ ATmega328P ──── UART (TX) ──── PC
  [AHT20]  ──── I2C (A4/A5) ─┘
```

### Schematic

```
                        Arduino Uno (ATmega328P @ 5V, 16MHz)
                       ┌─────────────────────────────────┐
                       │                                 │
          FC-22 ───────┤ A0 (ADC0)                       │
                       │                                 │
         BMP280 ───────┤ A4 (SDA) ──── 4.7kΩ ── 3.3V     │
         BMP280 ───────┤ A5 (SCL) ──── 4.7kΩ ── 3.3V     │
                       │                                 │
          AHT20 ───────┤ A4 (SDA)                        │
          AHT20 ───────┤ A5 (SCL)                        │
                       │                                 │
     BMP280 VCC ───────┤ 3.3V                            │
      AHT20 VCC ───────┤ 3.3V                            │
      FC-22 VCC ───────┤ 5V                              │
        All GND ───────┤ GND                             │
                       └─────────────────────────────────┘
```

### Electrical Parameters Comparison

Supply voltage:
 * ATmega328P: `1.8V – 5.5V`
 * BMP280: `1.71V – 3.6V`
 * AHT20: `2.0V – 5.5V`
 * FC-22: `5V`

Operating voltage:
 * ATmega328P: `5V`
 * BMP280: `3.3V`
 * AHT20: `3.3V`
 * FC-22: `5V`

Current Consumption:
 * ATmega328P: `~12 mA`
 * BMP280: `~2.7 µA`
 * AHT20: `~230 µA`
 * FC-22: `~150 mA`

I2C logic voltage:
 * ATmega328P: `5V`
 * BMP280: `3.3V` (receiving `5V`. Level shifters required)
 * AHT20: `3.3V` (receiving `5V`. Level shifters required)
 * FC-22: `-`

### Electrical Compatibility Discussion

BMP280 operates at 3.3V but receives 5V I2C signals from the ATmega328P.

The ATmega328P HIGH output threshold is ~0.7 × VCC = 3.5V, which exceeds
the BMP280's maximum SDA/SCL voltage of 3.6V. This is marginally safe but
level shifters are strongly recommended. 

The AHT20 supports up to 5.5V so
it is fully compatible with 5V I2C signals despite being powered at 3.3V.

The FC-22 runs at 5V matching the MCU — no compatibility issues.

### Power Consumption

Current consumptions:
  * ATmega328P @ 5V 16MHz: `12 mA`
  * BMP280 (normal mode): `0.003 mA`
  * AHT20 (measuring): `0.23 mA`
  * FC-22: `150 mA`
  * Arduino Uno board overhead (voltage regulator, USB, LEDs): `~50 mA`

Roughly `212mA`, powered at `5V` -> `1.06W`.

#### Practical Case

Cell specs:
  * Voltage: `1.2V`
  * Capacity: `2500 mAh`

Cells needed: ceil(5V / 1.2V) = `5 cells series` -> `6V`

**Runtime:** 2500mAh / 212mA = `~11.8 hrs`

> Capacity does not change when the cells are in series.

### Power Reduction

#### AVR Sleep Modes

Put the MCU into **power-down sleep** between readings:

```
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();  // wakes on interrupt or watchdog timer
```

#### Disable Unused Peripherals

```
  // Disable SPI, USART, TWI when not in use
  power_spi_disable();
  power_usart0_disable();
  power_twi_disable();
```

#### Replace FC-22 with SGP30

FC-22 draws 150mA (Analog). SGP30 draws 48mA (I2C digital gas sensor).

#### Reduce Sampling Rate

Obviously, the sampling rate can be reduced to read fewer times, being more energy consumption efficient.

## Notes

### More Sensors Used

The circuit uses a variety of different sensors:
 * FC-22 (analog, ADC): gas sensor
 * BMP280 digital:
   * temperature sensor
   * pressure sensor
 * AHT20 digital:
   * temperature sensor
   * humidity sensor

### Practical Industrial Use Case

This system could serve as a low-cost indoor air quality and
environmental monitoring station. The FC-22 detects gas leaks,
BMP280 monitors atmospheric pressure for weather prediction,
and AHT20 tracks temperature and humidity for HVAC control.
With sleep mode enabled, battery life exceeds 100 hours on a
standard 2500mAh pack, making it suitable for wireless deployment.

### AHT20 Sensor Used for Demonstration

AHT20 sensor might be broken or damaged. Humidity is stuck at 99.99%, exception being that sometimes when it is touched it drops.
The temperature seems to be working.
Still, it is being handled by the I2C protocol.

### Compiler Optimizations

Compiled with -Os (optimize for size) which reduces flash usage
and improves cache efficiency on the ATmega328P's limited 32KB flash.

### Sleep Implementation

Power-down sleep is used between readings via the watchdog timer,
reducing MCU current from 12mA to 0.1µA during the 500ms idle period.

### Others

The sampling rate can be changed in `utils.h`.

To compile, run `make` to generate executable, then `make flash` to upload file to MCU.

To run, make sure the Arduino is plugged in a device. To run on linux, use command `screen /dev/ttyUSBx 9600`.

To exit screen command, type `Ctrl + A + K`, then `y` and `enter`.
