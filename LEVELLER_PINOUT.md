# Leveller (Lindi) - Camper Levelling Indicator
## Project-Specific Pinout Configuration

**Project**: Camper Levelling Indicator
**Board**: Waveshare ESP32-C6-Zero
**Last Updated**: 2025-12-17

---

## Project Overview

The Leveller (Lindi) is a camper levelling indicator that displays the vehicle's tilt angle on an RGB LCD display. The system uses a 6-axis gyro/accelerometer to measure orientation and provides visual feedback with color-coded levelling status.

### Components List

| Component | Model/Type | Interface | Address/Details |
|-----------|------------|-----------|-----------------|
| LCD Display | Grove RGB Backlight LCD (JHD1313M3) | I2C | LCD: 0x3E, RGB: 0x62 |
| Gyro/Accelerometer | MPU6050 | I2C | 0x68 (default) or 0x69 |
| Mode Button 1 | Push Button | GPIO | Active low with pullup |
| Mode Button 2 | Push Button | GPIO | Active low with pullup |
| Status LED | WS2812B RGB | GPIO | Onboard LED |

---

## Pin Configuration

### I2C Bus (Primary)

**ESP32-C6 I2C Pins:**

| ESP32-C6 Pin | Function | Connected To |
|--------------|----------|--------------|
| GPIO6 | I2C SDA | LCD SDA + MPU6050 SDA |
| GPIO7 | I2C SCL | LCD SCL + MPU6050 SCL |
| 3V3 | Power | LCD VCC + MPU6050 VCC |
| GND | Ground | LCD GND + MPU6050 GND |

**I2C Device Addresses:**
- JHD1313M3 LCD: 0x3E
- JHD1313M3 RGB Controller: 0x62
- MPU6050 Gyro/Accel: 0x68 (AD0 low) or 0x69 (AD0 high)

---

## Component Connections

### 1. Grove RGB Backlight LCD (JHD1313M3)

**Display Specifications:**
- 16x2 character LCD
- RGB backlight control
- Dual I2C devices (LCD + RGB controller)

**Pin Connections:**

| LCD Pin | Wire Color | ESP32-C6 Pin | Notes |
|---------|------------|--------------|-------|
| GND | Black | GND | Ground |
| VCC | Red | 3V3 | 3.3V or 5V (check module) |
| SDA | White | GPIO6 | I2C Data |
| SCL | Yellow | GPIO7 | I2C Clock |

**I2C Configuration:**
```c
// LCD Text Controller
#define LCD_I2C_ADDR    0x3E

// RGB Backlight Controller
#define RGB_I2C_ADDR    0x62

// I2C Pins
#define I2C_SDA_PIN     GPIO_NUM_6
#define I2C_SCL_PIN     GPIO_NUM_7
#define I2C_FREQ_HZ     100000
```

---

### 2. MPU6050 Gyro/Accelerometer

**Sensor Specifications:**
- 3-axis gyroscope
- 3-axis accelerometer
- Digital Motion Processor (DMP)
- Temperature sensor

**Pin Connections:**

| MPU6050 Pin | ESP32-C6 Pin | Notes |
|-------------|--------------|-------|
| VCC | 3V3 | 3.3V power |
| GND | GND | Ground |
| SCL | GPIO7 | I2C Clock (shared) |
| SDA | GPIO6 | I2C Data (shared) |
| XDA | - | Not connected |
| XCL | - | Not connected |
| AD0 | GND | I2C address select (0x68) |
| INT | GPIO3 | Interrupt (optional) |

**I2C Configuration:**
```c
#define MPU6050_I2C_ADDR    0x68  // AD0 = GND

// Optional interrupt pin
#define MPU6050_INT_PIN     GPIO_NUM_3
```

---

### 3. Mode Control Buttons

**Button 1 - Mode Switch:**

| Button Pin | ESP32-C6 Pin | Configuration |
|------------|--------------|---------------|
| Pin 1 | GPIO4 | Input with internal pullup |
| Pin 2 | GND | Ground |

**Button 2 - Function Switch:**

| Button Pin | ESP32-C6 Pin | Configuration |
|------------|--------------|---------------|
| Pin 1 | GPIO5 | Input with internal pullup |
| Pin 2 | GND | Ground |

**GPIO Configuration:**
```c
#define MODE_BUTTON_PIN     GPIO_NUM_4
#define FUNC_BUTTON_PIN     GPIO_NUM_5

// Active LOW (pressed = 0, released = 1)
// Internal pullup enabled
```

---

### 4. Status LED (Onboard)

**Onboard RGB LED:**

| Component | ESP32-C6 Pin | Type |
|-----------|--------------|------|
| RGB LED | GPIO8 | WS2812B addressable |

**Configuration:**
```c
#define STATUS_LED_PIN      GPIO_NUM_8
#define LED_COUNT           1
```

---

## Pin Summary Table

| ESP32-C6 GPIO | Function | Connected To | Type |
|---------------|----------|--------------|------|
| GPIO3 | INT (optional) | MPU6050 INT | Input |
| GPIO4 | Button Input | Mode Button | Input (pullup) |
| GPIO5 | Button Input | Function Button | Input (pullup) |
| GPIO6 | I2C SDA | LCD + MPU6050 | Bidirectional |
| GPIO7 | I2C SCL | LCD + MPU6050 | Output |
| GPIO8 | WS2812B | Onboard RGB LED | Output |
| GPIO9 | Button | Onboard BOOT button | Input (pullup) |
| 3V3 | Power | All components VCC | Power |
| GND | Ground | All components GND | Ground |

---

## Power Budget

| Component | Voltage | Current (typ) | Current (max) | Notes |
|-----------|---------|---------------|---------------|-------|
| ESP32-C6 | 3.3V | 80mA | 350mA | Active WiFi |
| JHD1313M3 LCD | 3.3-5V | 50mA | 80mA | Backlight on |
| MPU6050 | 3.3V | 3.9mA | 500uA | Low power mode |
| WS2812B LED | 3.3V | 1mA | 60mA | Per LED max brightness |
| **Total** | 3.3V | ~135mA | ~490mA | USB powered OK |

---

## I2C Bus Configuration

**Bus Parameters:**
- **Speed**: 100kHz (standard mode)
- **Pullup Resistors**: Internal (or external 4.7kΩ if needed)
- **SDA**: GPIO6
- **SCL**: GPIO7

**ESP-IDF Configuration:**
```c
i2c_config_t i2c_config = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = GPIO_NUM_6,
    .scl_io_num = GPIO_NUM_7,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 100000,
};
```

---

## Operating Modes

### Mode 1: Live Level Display
- Real-time tilt angles (pitch/roll)
- Color-coded backlight (green = level, yellow = slight tilt, red = major tilt)
- Continuous update

### Mode 2: Calibration Mode
- Zero-point calibration
- Offset adjustment
- Save to NVS (non-volatile storage)

### Mode 3: Compass Mode
- Heading display using magnetometer (future enhancement)
- Directional indicator

### Mode 4: Diagnostics
- Sensor status
- I2C device scan
- Battery/power info

---

## LCD Display Layout

### Mode 1 - Live Level:
```
[Pitch: +2.5°  ]
[Roll:  -1.3°  ]
```

### Mode 2 - Calibration:
```
[CALIBRATING... ]
[Place on level ]
```

### Mode 3 - Compass:
```
[Heading: 273°  ]
[West           ]
```

### Mode 4 - Diagnostics:
```
[MPU: OK LCD:OK]
[Power:  USB 5V]
```

---

## RGB Backlight Color Coding

| Level Status | Backlight Color | Pitch/Roll Range |
|--------------|-----------------|------------------|
| Perfect Level | Green (0,255,0) | ±0.5° |
| Slightly Tilted | Yellow (255,255,0) | ±0.5° to ±2° |
| Moderately Tilted | Orange (255,128,0) | ±2° to ±5° |
| Severely Tilted | Red (255,0,0) | >±5° |

---

## Button Functions

| Button | Short Press | Long Press (>2s) | In Menu |
|--------|-------------|------------------|---------|
| Mode (GPIO4) | Next mode | Enter settings | Confirm |
| Function (GPIO5) | Cycle display | Reset calibration | Cancel |
| BOOT (GPIO9) | - | Factory reset | Back |

---

## Required ESP-IDF Components

### Core Components
- `driver/i2c` - I2C bus communication
- `driver/gpio` - GPIO control
- `driver/rmt` - WS2812B LED control
- `nvs_flash` - Non-volatile storage for calibration

### Third-Party Libraries

**1. JHD1313M3 LCD Library**
- Component name: `grove_lcd_rgb`
- I2C address: 0x3E (LCD), 0x62 (RGB)

**2. MPU6050 Library**
- Component name: `mpu6050`
- I2C address: 0x68

**3. WS2812B LED Library**
- Component name: `led_strip`
- RMT channel driver

---

## Wiring Diagram Notes

### Physical Layout
```
ESP32-C6-Zero
    ┌─────────────┐
    │   USB-C     │
    ├─────────────┤
3V3 │●           ●│ 5V
RST │●           ●│ GND
GP0 │●           ●│ GP23
GP1 │●           ●│ GP22
GP2 │●           ●│ GP21
GP3 │●───INT─────●│ GP20  (MPU6050 optional)
GP4 │●───BTN1────●│ GP19  (Mode button)
GP5 │●───BTN2────●│ GP18  (Function button)
GP6 │●───SDA─────●│ GP15  (I2C Data)
GP7 │●───SCL─────●│ GP9   (I2C Clock)
GND │●           ●│ GP8   (Onboard LED)
    └─────────────┘

I2C Bus:
GP6 (SDA) ──┬── LCD SDA
            └── MPU6050 SDA

GP7 (SCL) ──┬── LCD SCL
            └── MPU6050 SCL
```

---

## Development Checklist

- [ ] I2C bus initialization and device scan
- [ ] JHD1313M3 LCD driver integration
- [ ] MPU6050 sensor driver integration
- [ ] Calibration routine (zero-point offset)
- [ ] Angle calculation (pitch/roll from accelerometer)
- [ ] RGB backlight color mapping
- [ ] Button debouncing and mode switching
- [ ] NVS storage for calibration data
- [ ] Low-power sleep mode (when idle)
- [ ] Error handling and diagnostics display

---

## References

- [ESP32-C6 I2C Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/i2c.html)
- [MPU6050 Datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
- [JHD1313M3 LCD Datasheet](http://www.seeedstudio.com/wiki/Grove_-_LCD_RGB_Backlight)
- [WS2812B LED Datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)
