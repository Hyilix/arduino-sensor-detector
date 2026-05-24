# Project settings
TARGET  = main
MCU     = atmega328p
AVRMCU  = m328p
F_CPU   = 16000000UL
BAUD    = 9600
PORT    = /dev/ttyUSB0

# Tools
CC      = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

# Flags
CFLAGS  = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DBAUD=$(BAUD) -Os -Wall

# Source files
SRCS    = src/main.c src/i2c.c src/adc.c src/uart.c src/bmp.c src/aht.c src/sleep.c
OBJS    = $(SRCS:.c=.o)

# Default target
all: $(TARGET).hex

# Compile each .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link all .o into .elf
$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Convert .elf to .hex
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

# Flash to Arduino
flash: $(TARGET).hex
	$(AVRDUDE) -c arduino -p $(AVRMCU) -P $(PORT) -b 115200 -U flash:w:$(TARGET).hex

# Remove build artifacts
clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).hex

.PHONY: all flash clean
