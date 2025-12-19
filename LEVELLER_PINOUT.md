# Leveller (Lindi) - Camper Levelling Indicator
## Project-Specific Pinout Configuration

**Project**: Camper Levelling Indicator
**Board**: ESP32-SCREEN (Marauder Board) with ESP32-WROOM-32
**Display**: 2.4" TFT LCD with Touch Controller (SPI Interface)
**Last Updated**: 2025-12-19

---

## Project Overview

The Leveller (Lindi) is a camper levelling indicator that displays the vehicle's tilt angle on a TFT LCD display. The system uses a 6-axis gyro/accelerometer (MPU6050) to measure orientation and a GPS module for heading/position information, with visual feedback on the color TFT screen.

### Components List

| Component | Model/Type | Interface | Address/Details |
|-----------|------------|-----------|-----------------|
| TFT LCD Display | 2.4" TFT (ILI9341/ST7789) | SPI | CS=IO13, DC=IO15, RST=IO14, MOSI=IO12, SCK=IO2 |
| Touch Controller | XPT2046 | SPI | CS=IO33, IRQ=IO39, MOSI=IO32, MISO=IO36, SCK=IO25 |
| Gyro/Accelerometer | MPU6050 | I2C | 0x68 (default) or 0x69 (via P2 header) |
| GPS Module | GY-GPS6MV2 | UART | TX=IO27, RX=IO35 |
| RGB LED | WS2812B-like | GPIO | R=IO17, G=IO4, B=IO16 |
| Speaker | SC8002B Audio Amp | PWM | IO26 |

---

## ESP32-WROOM-32 Pin Allocation

### Already Used Pins (Built-in Hardware)

#### TFT LCD Display (SPI)
| Function | ESP32 Pin | Description |
|----------|-----------|-------------|
| TFT_RST | IO14 | Display reset |
| TFT_SCK | IO2 | SPI clock |
| TFT_RS (DC) | IO15 | Data/Command select |
| TFT_CS | IO13 | Chip select |
| TFT_SDI (MOSI) | IO12 | SPI data out |
| TFT_SDO (MISO) | - | Not used (display only) |

#### Touch Controller (XPT2046 - Separate SPI)
| Function | ESP32 Pin | Description |
|----------|-----------|-------------|
| TP_CLK | IO25 | Touch SPI clock |
| TP_CS | IO33 | Touch chip select |
| TP_DIN (MOSI) | IO32 | Touch data in |
| TP_OUT (MISO) | IO36 | Touch data out |
| TP_IRQ | IO39 | Touch interrupt |

#### Audio Output
| Function | ESP32 Pin | Description |
|----------|-----------|-------------|
| SPEAKER | IO26 | PWM audio output (SC8002B amp) |

#### RGB LED
| Function | ESP32 Pin | Description |
|----------|-----------|-------------|
| LED_RED | IO17 | Red LED channel |
| LED_GREEN | IO4 | Green LED channel |
| LED_BLUE | IO16 | Blue LED channel |

#### System/Reserved
| Function | ESP32 Pin | Description |
|----------|-----------|-------------|
| U0TXD | GPIO1 | USB serial transmit |
| U0RXD | GPIO3 | USB serial receive |
| EN | - | Reset/Enable |

### Available Pins for External Sensors

#### External I/O Header P2 (4-pin connector)
| Pin | ESP32 GPIO | Function | Available For |
|-----|------------|----------|---------------|
| 1 | IO21 | I2C SDA | MPU6050 SDA |
| 2 | IO22 | I2C SCL | MPU6050 SCL |
| 3 | IO35 | UART RX | GPS TX (crossover) |
| 4 | GND | Ground | Common ground |

#### Additional Available GPIOs
| ESP32 Pin | Status | Recommended Use |
|-----------|--------|-----------------|
| IO27 | Available | GPS RX (UART TX from ESP32) |
| IO0 | Available (boot strap) | User button / calibration trigger |
| IO5 | Available | Reserved for future expansion |
| IO18 | Available | Reserved for future expansion |
| IO19 | Available | Reserved for future expansion |
| IO21 | Available (P2 header) | MPU6050 SDA |
| IO22 | Available (P2 header) | MPU6050 SCL |
| IO23 | Available | Reserved for future expansion |
| IO34 | Input only | Analog sensor input |
| IO35 | Available (P2 header) | GPS UART RX |

---

## Component Connections

### 1. MPU6050 Gyro/Accelerometer (via P2 Header)

**Sensor Specifications:**
- 3-axis gyroscope (pitch, roll, yaw)
- 3-axis accelerometer
- Digital Motion Processor (DMP)
- Temperature sensor
- I2C interface (400kHz capable)

**Pin Connections:**

| MPU6050 Pin | ESP32 Pin | P2 Header Pin | Notes |
|-------------|-----------|---------------|-------|
| VCC | 3.3V | - | 3.3V power |
| GND | GND | Pin 4 | Ground |
| SCL | IO22 | Pin 2 | I2C Clock |
| SDA | IO21 | Pin 1 | I2C Data |
| AD0 | GND | - | I2C address select (0x68) |
| INT | IO34 | - | Interrupt (optional) |

**I2C Configuration:**
```c
#define MPU6050_I2C_ADDR    0x68  // AD0 = GND
#define I2C_SDA_PIN         GPIO_NUM_21
#define I2C_SCL_PIN         GPIO_NUM_22
#define I2C_FREQ_HZ         100000  // 100kHz (can go up to 400kHz)

// Optional interrupt pin
#define MPU6050_INT_PIN     GPIO_NUM_34  // Input only pin
```

---

### 2. GPS Module (GY-GPS6MV2)

**GPS Module Specifications:**
- NEO-6M GPS module
- UART interface (default 9600 baud)
- NMEA protocol output
- 3.3V or 5V compatible (check module)

**Pin Connections:**

| GPS Module Pin | ESP32 Pin | Notes |
|----------------|-----------|-------|
| VCC | 3.3V or 5V | Check module voltage requirement |
| GND | GND | Ground |
| TX | IO35 | GPS transmit → ESP32 receive (P2 Pin 3) |
| RX | IO27 | GPS receive ← ESP32 transmit |

**UART Configuration:**
```c
#define GPS_UART_NUM        UART_NUM_1  // UART1 (UART0 used by USB)
#define GPS_TX_PIN          GPIO_NUM_27  // ESP32 TX → GPS RX
#define GPS_RX_PIN          GPIO_NUM_35  // ESP32 RX ← GPS TX
#define GPS_BAUD_RATE       9600         // Standard GPS baud rate
#define GPS_RX_BUFFER_SIZE  1024
```

**Note**: GPS TX connects to ESP32 RX, and GPS RX connects to ESP32 TX (crossover connection).

---

### 3. TFT LCD Display (Built-in - SPI)

**Display Specifications:**
- 2.4" TFT LCD (likely ILI9341 or ST7789 controller)
- 320x240 resolution (typical)
- 16-bit color (RGB565)
- SPI interface
- Backlight control via power rail

**Pin Connections (Built-in):**

| TFT Signal | ESP32 Pin | SPI Function | Notes |
|------------|-----------|--------------|-------|
| TFT_RST | IO14 | Reset | Active low |
| TFT_SCK | IO2 | SPI CLK | SPI clock |
| TFT_RS (DC) | IO15 | Data/Command | 0=command, 1=data |
| TFT_CS | IO13 | Chip Select | Active low |
| TFT_SDI (MOSI) | IO12 | SPI MOSI | Data to display |
| VDD | 3.3V | Power | 3.3V rail |
| GND | GND | Ground | Common ground |

**SPI Configuration:**
```c
#define TFT_SPI_HOST        SPI2_HOST
#define TFT_MOSI_PIN        GPIO_NUM_12
#define TFT_CLK_PIN         GPIO_NUM_2
#define TFT_CS_PIN          GPIO_NUM_13
#define TFT_DC_PIN          GPIO_NUM_15
#define TFT_RST_PIN         GPIO_NUM_14
#define TFT_SPI_FREQ_HZ     40000000  // 40MHz SPI clock
```

**Display Driver Options:**
- LVGL (Light and Versatile Graphics Library) - Recommended
- ESP_LCD component (ESP-IDF native)
- TFT_eSPI library (Arduino-style)

---

### 4. Touch Controller (XPT2046 - Built-in)

**Touch Specifications:**
- 4-wire resistive touch
- SPI interface (separate from LCD)
- Interrupt-driven or polling mode

**Pin Connections (Built-in):**

| Touch Signal | ESP32 Pin | Notes |
|--------------|-----------|-------|
| TP_CLK | IO25 | Touch SPI clock |
| TP_CS | IO33 | Touch chip select |
| TP_DIN (MOSI) | IO32 | Data to touch controller |
| TP_OUT (MISO) | IO36 | Data from touch controller |
| TP_IRQ | IO39 | Touch interrupt (active low) |

**SPI Configuration:**
```c
#define TOUCH_SPI_HOST      SPI3_HOST  // Separate SPI bus
#define TOUCH_MOSI_PIN      GPIO_NUM_32
#define TOUCH_MISO_PIN      GPIO_NUM_36
#define TOUCH_CLK_PIN       GPIO_NUM_25
#define TOUCH_CS_PIN        GPIO_NUM_33
#define TOUCH_IRQ_PIN       GPIO_NUM_39
#define TOUCH_SPI_FREQ_HZ   2000000  // 2MHz for touch
```

---

### 5. RGB LED (Built-in)

**LED Configuration:**
| Color | ESP32 Pin | Type |
|-------|-----------|------|
| Red | IO17 | GPIO output |
| Green | IO4 | GPIO output |
| Blue | IO16 | GPIO output |

**GPIO Configuration:**
```c
#define LED_RED_PIN     GPIO_NUM_17
#define LED_GREEN_PIN   GPIO_NUM_4
#define LED_BLUE_PIN    GPIO_NUM_16

// Active high - set pin HIGH to turn LED on
```

---

### 6. Speaker (Built-in)

**Audio Output:**
| Function | ESP32 Pin | Amplifier | Notes |
|----------|-----------|-----------|-------|
| Speaker | IO26 | SC8002B | PWM audio output |

**PWM/Audio Configuration:**
```c
#define SPEAKER_PIN     GPIO_NUM_26
// Use LEDC PWM or DAC for audio generation
```

---

## Complete Pin Summary Table

| ESP32 GPIO | Function | Component | Type | Notes |
|------------|----------|-----------|------|-------|
| GPIO1 | U0TXD | USB Serial | Output | Reserved |
| GPIO2 | TFT_SCK | TFT Display | SPI CLK | Built-in |
| GPIO3 | U0RXD | USB Serial | Input | Reserved |
| GPIO4 | LED_GREEN | RGB LED | Output | Built-in |
| GPIO12 | TFT_MOSI | TFT Display | SPI MOSI | Built-in |
| GPIO13 | TFT_CS | TFT Display | SPI CS | Built-in |
| GPIO14 | TFT_RST | TFT Display | Output | Built-in |
| GPIO15 | TFT_DC | TFT Display | Output | Built-in |
| GPIO16 | LED_BLUE | RGB LED | Output | Built-in |
| GPIO17 | LED_RED | RGB LED | Output | Built-in |
| GPIO21 | I2C_SDA | MPU6050 | I2C Data | P2 Pin 1 |
| GPIO22 | I2C_SCL | MPU6050 | I2C Clock | P2 Pin 2 |
| GPIO25 | TP_CLK | Touch | SPI CLK | Built-in |
| GPIO26 | SPEAKER | Audio Amp | PWM Out | Built-in |
| GPIO27 | GPS_TX | GPS Module | UART TX | Available |
| GPIO32 | TP_MOSI | Touch | SPI MOSI | Built-in |
| GPIO33 | TP_CS | Touch | SPI CS | Built-in |
| GPIO34 | MPU_INT | MPU6050 (opt) | Input Only | Optional |
| GPIO35 | GPS_RX | GPS Module | UART RX | P2 Pin 3 |
| GPIO36 | TP_MISO | Touch | SPI MISO | Built-in |
| GPIO39 | TP_IRQ | Touch | Input Only | Built-in |
| GPIO0 | - | Available | Input/Output | Boot strap |
| GPIO5 | - | Available | Input/Output | Future use |
| GPIO18 | - | Available | Input/Output | Future use |
| GPIO19 | - | Available | Input/Output | Future use |
| GPIO23 | - | Available | Input/Output | Future use |

---

## Power Budget

| Component | Voltage | Current (typ) | Current (max) | Notes |
|-----------|---------|---------------|---------------|-------|
| ESP32-WROOM-32 | 3.3V | 80mA | 350mA | Active WiFi/BT |
| TFT LCD 2.4" | 3.3V | 40mA | 100mA | Backlight on |
| XPT2046 Touch | 3.3V | 200µA | 1mA | Active mode |
| MPU6050 | 3.3V | 3.9mA | 500µA (sleep) | Normal operation |
| GPS GY-GPS6MV2 | 3.3-5V | 45mA | 67mA | Tracking mode |
| RGB LED | 3.3V | 20mA | 60mA | All channels max |
| SC8002B Audio | 3.3V | 1mA | 200mA | Speaker playing |
| **Total** | 3.3V | ~190mA | ~778mA | USB powered OK |

**Note**: USB 2.0 provides 500mA minimum. Total max current is within limits. Consider low-power modes for battery operation.

---

## I2C Bus Configuration

**Bus Parameters:**
- **Speed**: 100kHz standard mode (MPU6050 supports up to 400kHz)
- **Pullup Resistors**: Internal ESP32 pullups enabled (or external 4.7kΩ if needed)
- **SDA**: GPIO21 (P2 header pin 1)
- **SCL**: GPIO22 (P2 header pin 2)
- **Devices**: MPU6050 at 0x68

**ESP-IDF Configuration:**
```c
i2c_config_t i2c_config = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = GPIO_NUM_21,
    .scl_io_num = GPIO_NUM_22,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 100000,  // 100kHz
};

i2c_param_config(I2C_NUM_0, &i2c_config);
i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
```

---

## UART Configuration for GPS

**UART Parameters:**
- **Port**: UART1 (UART0 used by USB serial)
- **Baud Rate**: 9600 (standard GPS)
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1
- **TX**: GPIO27
- **RX**: GPIO35

**ESP-IDF Configuration:**
```c
uart_config_t uart_config = {
    .baud_rate = 9600,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
};

uart_param_config(UART_NUM_1, &uart_config);
uart_set_pin(UART_NUM_1, GPIO_NUM_27, GPIO_NUM_35, -1, -1);
uart_driver_install(UART_NUM_1, 1024, 0, 0, NULL, 0);
```

---

## Operating Modes

### Mode 1: Live Level Display
- Real-time tilt angles (pitch/roll from MPU6050)
- Color-coded visual indicator on TFT
- Heading from GPS (when available)
- Continuous update at 10Hz

### Mode 2: Calibration Mode
- Zero-point calibration for MPU6050
- Offset adjustment and storage
- Save to NVS (non-volatile storage)
- Visual feedback on TFT

### Mode 3: GPS Navigation Mode
- GPS coordinates display
- Heading and speed from GPS
- Satellite count and signal strength
- Position logging

### Mode 4: Diagnostics
- Sensor status (MPU6050, GPS)
- I2C device scan
- GPS satellite info
- Battery/power status

---

## TFT Display Layout Examples

### Mode 1 - Live Level:
```
┌────────────────────────┐
│  LEVELLER - LIVE       │
├────────────────────────┤
│ Pitch: +2.5°  ↗        │
│ Roll:  -1.3°  ↖        │
│                        │
│ [   Bubble Level   ]   │
│        Graphic         │
│                        │
│ GPS: 273° W  45 km/h   │
└────────────────────────┘
```

### Mode 2 - Calibration:
```
┌────────────────────────┐
│  CALIBRATION MODE      │
├────────────────────────┤
│                        │
│   Place on Level       │
│      Surface           │
│                        │
│   Press OK when        │
│      Ready             │
│                        │
│ [Cancel]  [Calibrate]  │
└────────────────────────┘
```

### Mode 3 - GPS Info:
```
┌────────────────────────┐
│  GPS NAVIGATION        │
├────────────────────────┤
│ Sats: 8   HDOP: 1.2    │
│ Lat:  -33.7456°        │
│ Lon:  151.2347°        │
│                        │
│ Heading: 273° (W)      │
│ Speed:   45.3 km/h     │
│ Alt:     125m          │
└────────────────────────┘
```

---

## Wiring Diagram Notes

### Physical Layout
```
ESP32-SCREEN Board
┌──────────────────────────┐
│      TFT Display         │
│      (Built-in)          │
├──────────────────────────┤
│  [Touch Screen Active]   │
├──────────────────────────┤
│ RGB LED  [●]  SPEAKER    │
│                          │
│ P2 External Header:      │
│  1: IO21 (SDA)  ───────→ MPU6050 SDA
│  2: IO22 (SCL)  ───────→ MPU6050 SCL
│  3: IO35 (RX)   ───────→ GPS TX
│  4: GND         ───────→ Common GND
│                          │
│ IO27 (TX) ──────────────→ GPS RX
│                          │
│      USB-C Port          │
└──────────────────────────┘
```

**External Connections:**
```
P2 Header (4-pin):
┌─────┬─────┬──────┬─────┐
│ IO21│ IO22│ IO35 │ GND │
└──┬──┴──┬──┴───┬──┴──┬──┘
   │     │      │     │
   │     │      │     └─────→ MPU6050 GND, GPS GND
   │     │      └───────────→ GPS TX (NEO-6M TX)
   │     └──────────────────→ MPU6050 SCL
   └────────────────────────→ MPU6050 SDA

GPIO27 (separate wire) ────→ GPS RX (NEO-6M RX)
```

---

## Required ESP-IDF Components

### Core Components
- `driver/i2c` - I2C bus for MPU6050
- `driver/uart` - UART for GPS
- `driver/gpio` - GPIO control for LED
- `driver/spi_master` - SPI for TFT and Touch
- `esp_lcd` - LCD driver framework
- `lvgl` - Graphics library (recommended)
- `nvs_flash` - Non-volatile storage for calibration

### Third-Party Libraries (via idf_component.yml)

**1. MPU6050 Library**
```yaml
dependencies:
  mpu6050:
    git: https://github.com/natanaeljr/esp32-MPU-driver.git
```

**2. LVGL (Graphics)**
```yaml
dependencies:
  lvgl/lvgl:
    version: "^8.3.0"
```

**3. GPS NMEA Parser**
```yaml
dependencies:
  nmea_parser:
    git: https://github.com/espressif/esp-iot-solution.git
    path: components/gps/nmea_parser
```

---

## Development Checklist

- [ ] SPI bus initialization for TFT display
- [ ] TFT display driver integration (ILI9341/ST7789)
- [ ] LVGL graphics library setup
- [ ] Touch controller (XPT2046) driver integration
- [ ] I2C bus initialization for MPU6050
- [ ] MPU6050 sensor driver integration
- [ ] UART initialization for GPS
- [ ] GPS NMEA parser integration
- [ ] Calibration routine (zero-point offset for MPU6050)
- [ ] Angle calculation (pitch/roll from accelerometer)
- [ ] GPS data parsing and display
- [ ] UI design with LVGL (level display, calibration screens)
- [ ] Touch input handling
- [ ] NVS storage for calibration data
- [ ] Low-power sleep mode (when idle)
- [ ] Error handling and diagnostics display

---

## References

- [ESP32 SPI Master Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/spi_master.html)
- [ESP32 I2C Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html)
- [ESP32 UART Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html)
- [ESP_LCD Component](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/lcd.html)
- [LVGL Documentation](https://docs.lvgl.io/)
- [MPU6050 Datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
- [XPT2046 Touch Controller Datasheet](https://www.buydisplay.com/download/ic/XPT2046.pdf)
- [NEO-6M GPS Datasheet](https://www.u-blox.com/sites/default/files/products/documents/NEO-6_DataSheet_(GPS.G6-HW-09005).pdf)
- [ILI9341 TFT Controller](https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf)

---

## Board Source and Configuration

**Board Origin**: Copied from Marauder project (E:\Dev\Marauder)
**Schematic**: `boards/marauder/docs/ESP32_CREEN.pdf`
**Components**: `boards/marauder/components/` (TFT, touch, GUI drivers)
**sdkconfig**: Root `sdkconfig` matches Marauder board (backup: `sdkconfig.esp32c6.bak`)

---

**Last Updated**: 2025-12-19
**Board Version**: ESP32-SCREEN (Marauder) v1.0
**ESP-IDF Version**: v5.5
