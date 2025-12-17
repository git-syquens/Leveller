# Waveshare ESP32-C6-Zero Pinout Reference

## Board Overview

The Waveshare ESP32-C6-Zero is a compact development board featuring the ESP32-C6FH4 chip with RISC-V 32-bit single-core processor.

### Core Specifications

- **Chip**: ESP32-C6FH4
- **Processor**: RISC-V 32-bit single-core, up to 160 MHz
- **Memory**:
  - 320KB ROM
  - 512KB HP SRAM
  - 16KB LP SRAM
  - 4MB Flash
- **Wireless**: WiFi 6, Bluetooth 5, IEEE 802.15.4 (Zigbee 3.0, Thread)
- **USB**: Type-C connector with automatic download circuit
- **Form Factor**: Castellated module for direct PCB soldering

## Pin Layout

![Pinout Diagram](https://www.waveshare.com/w/upload/2/2e/ESP32-C6-Zero_Pin.png)

### Left Side (Top to Bottom)

| Pin | Function | Description |
|-----|----------|-------------|
| 3V3 | Power | 3.3V Output/Input |
| RST | Reset | Reset pin (active low) |
| GPIO0 | GPIO/ADC | General purpose I/O / ADC |
| GPIO1 | GPIO/ADC | General purpose I/O / ADC |
| GPIO2 | GPIO/ADC | General purpose I/O / ADC |
| GPIO3 | GPIO/ADC | General purpose I/O / ADC |
| GPIO4 | GPIO/UART | General purpose I/O / UART TX (example use) |
| GPIO5 | GPIO/UART | General purpose I/O / UART RX (example use) |
| GPIO6 | GPIO | General purpose I/O |
| GPIO7 | GPIO | General purpose I/O |
| GND | Ground | Ground |

### Right Side (Top to Bottom)

| Pin | Function | Description |
|-----|----------|-------------|
| 5V | Power | 5V Input from USB |
| GND | Ground | Ground |
| GPIO23 | GPIO | General purpose I/O |
| GPIO22 | GPIO | General purpose I/O |
| GPIO21 | GPIO | General purpose I/O |
| GPIO20 | GPIO | General purpose I/O |
| GPIO19 | GPIO | General purpose I/O |
| GPIO18 | GPIO | General purpose I/O |
| GPIO15 | GPIO | General purpose I/O |
| GPIO9 | GPIO/BOOT | General purpose I/O / Boot button |
| GPIO8 | GPIO/RGB LED | General purpose I/O / Onboard RGB LED |

## Onboard Components

### RGB LED
- **GPIO**: GPIO8
- **Type**: WS2812B addressable RGB LED
- **Control**: Use with Blink example (get-started -> blink)

### BOOT Button
- **GPIO**: GPIO9
- **Function**:
  - Boot mode selection during reset
  - User button in normal operation
  - Used in Zigbee examples for control

### USB Interface
- **Type**: USB Type-C
- **Features**:
  - Built-in USB-to-JTAG/Serial bridge
  - Automatic download circuit (no manual boot button press needed)
  - JTAG debugging support
- **COM Port**: Assigned by system (check Device Manager on Windows)

## Peripheral Interface Mapping

### UART
- **Default Example Pins**: GPIO4 (TX), GPIO5 (RX)
- **Alternative**: USB CDC (via USB-C)

### SPI
- Available on multiple GPIO pins (configurable)

### I2C
- Available on multiple GPIO pins (configurable)

### ADC
- Available on GPIO0-3 (check ESP32-C6 datasheet for exact channels)

### PWM
- Available on most GPIO pins

## Power Supply

| Pin | Voltage | Max Current | Notes |
|-----|---------|-------------|-------|
| 5V | 5.0V | - | Input from USB Type-C |
| 3V3 | 3.3V | 500mA | Regulated output from onboard LDO |
| GND | 0V | - | Ground reference |

## GPIO Voltage Levels

- **Logic Level**: 3.3V
- **5V Tolerant**: NO - Do not apply 5V to GPIO pins

## Pin Usage Guidelines

### Safe Pins (Generally Available)
- GPIO0-7, GPIO15, GPIO18-23
- These can be used for most applications

### Special Function Pins
- **GPIO8**: Controls onboard RGB LED (can be used for other purposes if LED not needed)
- **GPIO9**: Connected to BOOT button (can be used with external pullup)

### Reserved/Internal Pins
- USB D+/D- are handled internally by the chip
- SPI Flash pins are used internally (do not use)

## Example Applications

### UART Communication
```c
// Hardware Connection
GPIO4 <-> GPIO5 (loopback test)

// Used in: peripherals/uart/uart_async_rxtxtasks
```

### RGB LED Control
```c
// GPIO8 controls WS2812B LED
// Used in: get-started/blink
```

### Zigbee Network
```c
// GPIO9 (BOOT button) used for network pairing
// Used in: zigbee/light_sample examples
```

## Physical Dimensions

- **Board Size**: Compact form factor with castellated edges
- **Mounting**: Can be soldered directly to carrier boards
- **Pin Pitch**: 2.54mm (0.1") standard spacing

## Development Resources

### Official Examples
- **Path**: ESP-IDF examples
- **Location**: Available via ESP-IDF extension in VSCode
- **Categories**:
  - Basic (hello_world, blink)
  - Peripherals (UART, SPI, I2C)
  - Wireless (WiFi, Bluetooth, Zigbee)
  - Advanced (JTAG debugging)

### Useful Links
- [Waveshare Wiki](https://www.waveshare.com/wiki/ESP32-C6-Zero)
- [ESP32-C6 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf)
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/)

## Notes

1. **First Flash**: Use USB interface (UART interface also works but USB is recommended)
2. **COM Port**: Check Device Manager (Windows) for assigned port
3. **JTAG Debug**: Only available through USB interface, not UART
4. **Low Power**: Supports multiple low-power modes (see ESP32-C6 documentation)
5. **Wireless Coexistence**: WiFi 6, Bluetooth 5, and IEEE 802.15.4 can operate simultaneously

## Project-Specific Pin Assignments

### Leveller Project

*Document your pin usage here as the project develops*

| GPIO | Function | Connected To | Notes |
|------|----------|--------------|-------|
| GPIO8 | Status LED | Onboard RGB | Default |
| GPIO9 | User Button | Onboard BOOT | Default |
| - | - | - | Add assignments as needed |

---

**Board**: Waveshare ESP32-C6-Zero
**Last Updated**: 2025-12-17
**Reference**: [Waveshare Wiki](https://www.waveshare.com/wiki/ESP32-C6-Zero)
