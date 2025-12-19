 ESP32 Development Board Pinout Reference

This document contains the pinout reference for the ESP32 development boards used in this project.

## Board Layout Overview

The attached image shows a comprehensive ESP32 development board pinout with the following key features:

### Power Pins
- **3V3** - 3.3V power output
- **GND** - Ground pins (multiple locations)
- **EN** - Enable pin
- **5V** - 5V power input

### GPIO Pins (Left Side)
- **GPIO34** (VP/SENSOR_VP) - Analog input only, no pull-up/pull-down
- **GPIO35** (VN/SENSOR_VN) - Analog input only, no pull-up/pull-down  
- **GPIO32** - Touch sensor, ADC1_CH4, DAC1
- **GPIO33** - Touch sensor, ADC1_CH5, DAC2
- **GPIO25** - DAC1, ADC2_CH8
- **GPIO26** - DAC2, ADC2_CH9
- **GPIO27** - Touch sensor, ADC2_CH7

### Special Function Pins (Left Side)
- **GPIO14** (MTMS) - Touch sensor, ADC2_CH6, HSPI_CLK
- **GPIO12** (MTDI) - Touch sensor, ADC2_CH5, HSPI_MISO
- **GPIO13** (MTCK) - Touch sensor, ADC2_CH4, HSPI_MOSI
- **D2** (SHD/SD2/SD_DATA_2) - SD card data line 2
- **D3** (SHD/SD3/SD_DATA_3) - SD card data line 3
- **CD_CMD** (SCS/CMD) - SD card command line

### GPIO Pins (Right Side)
- **GPIO23** - SPI MOSI
- **GPIO22** - I2C SCL
- **TX** (U0TXD) - UART0 transmit
- **RX** (U0RXD) - UART0 receive
- **GPIO21** - I2C SDA
- **GPIO19** - SPI MISO
- **GPIO18** - SPI CLK
- **GPIO5** - SPI CS
- **GPIO17** - UART2 TXD
- **GPIO16** - UART2 RXD
- **GPIO4** - Touch sensor, ADC2_CH0
- **GPIO0** - Boot button, pull-up required
- **GPIO2** - Built-in LED (blue), ADC2_CH2, touch sensor
- **GPIO15** (MTDO) - Touch sensor, ADC2_CH3, HSPI_CS

### SD Card Interface (Right Side)
- **SD_SATA_1** (SDO/SD0) - SD card data out
- **SD_DATA_0** (SDI/SD1) - SD card data in
- **CD_CLK** (SDK/SLK) - SD card clock

## Pin Categories

### Input Only Pins
- GPIO34, GPIO35 - These pins can only be used as inputs and do not have internal pull-up/pull-down resistors

### Touch Sensors
- GPIO0, GPIO2, GPIO4, GPIO12, GPIO13, GPIO14, GPIO15, GPIO27, GPIO32, GPIO33

### ADC Pins
- **ADC1**: GPIO32-GPIO39 (channels 0-7)
- **ADC2**: GPIO0, GPIO2, GPIO4, GPIO12-GPIO15, GPIO25-GPIO27 (channels 0-9)

### DAC Pins
- **DAC1**: GPIO25
- **DAC2**: GPIO26

### Communication Interfaces

#### SPI
- **MOSI**: GPIO23
- **MISO**: GPIO19 
- **CLK**: GPIO18
- **CS**: GPIO5

#### I2C
- **SDA**: GPIO21
- **SCL**: GPIO22

#### UART
- **UART0**: TX (GPIO1), RX (GPIO3)
- **UART2**: GPIO17 (TXD), GPIO16 (RXD)

## Special Notes

### Boot and Flash Pins
- **GPIO0**: Must be HIGH during boot for normal operation
- **GPIO2**: Must be floating or HIGH during boot
- **GPIO15**: Must be LOW during boot

### Strapping Pins
These pins have specific requirements during boot:
- GPIO0, GPIO2, GPIO5, GPIO12, GPIO15

### WiFi Usage
- When WiFi is active, ADC2 pins cannot be used for analog reading

## Usage Guidelines

1. **For WiFi projects**: Avoid ADC2 pins (GPIO0, GPIO2, GPIO4, GPIO12-GPIO15, GPIO25-GPIO27) for analog input
2. **For reliable boot**: Be careful with strapping pins
3. **For analog input**: Prefer ADC1 pins (GPIO32-GPIO39)
4. **For digital I/O**: Most GPIO pins can be used, but avoid input-only pins for output

## Project Context

This pinout reference is for the ESP32 development boards used in the WiFi security project with XOR obfuscated password storage.

Last Updated: October 3, 2025