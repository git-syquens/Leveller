# Leveller (Lindi) - Camper Levelling Indicator

ESP32-C6 based camper levelling indicator with RGB LCD display and 6-axis sensor.

## Hardware

- **MCU**: Waveshare ESP32-C6-Zero
- **Display**: Grove RGB Backlight LCD (JHD1313M3) - 16x2 with RGB backlight
- **Sensor**: MPU6050 6-axis gyroscope/accelerometer
- **Controls**: 2x push buttons for mode switching
- **Interface**: I2C (GPIO6=SDA, GPIO7=SCL)

## Features

- Real-time pitch and roll angle display
- Color-coded levelling status:
  - 🟢 Green: Perfect level (±0.5°)
  - 🟡 Yellow: Slight tilt (±0.5° to ±2°)
  - 🟠 Orange: Moderate tilt (±2° to ±5°)
  - 🔴 Red: Severe tilt (>±5°)
- Multiple display modes
- Calibration storage in NVS
- Low power consumption

## Pinout

See [LEVELLER_PINOUT.md](LEVELLER_PINOUT.md) for detailed pin connections.

| Component | ESP32-C6 Pin | Notes |
|-----------|--------------|-------|
| I2C SDA | GPIO6 | LCD + MPU6050 |
| I2C SCL | GPIO7 | LCD + MPU6050 |
| Mode Button | GPIO4 | Active low with pullup |
| Func Button | GPIO5 | Active low with pullup |
| Status LED | GPIO8 | Onboard WS2812B |

## Building

### Requirements

- ESP-IDF v5.1 or later
- VSCode with Espressif IDF extension (recommended)

### Setup

1. Clone the repository:
```bash
git clone https://github.com/git-syquens/Leveller.git
cd Leveller
```

2. Set ESP32-C6 as target:
```bash
idf.py set-target esp32c6
```

3. Build:
```bash
idf.py build
```

4. Flash and monitor:
```bash
idf.py -p COM4 flash monitor
```

## Project Structure

```
Leveller/
├── components/
│   ├── jhd1313m3/           # Custom LCD driver
│   │   ├── include/
│   │   │   └── jhd1313m3.h
│   │   ├── jhd1313m3.c
│   │   └── CMakeLists.txt
│   └── idf_component.yml    # Component dependencies
├── main/
│   ├── main.c               # Main application
│   └── CMakeLists.txt
├── CMakeLists.txt
├── LEVELLER_PINOUT.md       # Project-specific pinout
├── WAVESHARE_ESP32-C6-ZERO_PINOUT.md  # Board reference
└── README.md
```

## Usage

### Operating Modes

1. **Live Level Display** - Real-time pitch/roll angles
2. **Calibration Mode** - Zero-point calibration
3. **Compass Mode** - Heading display (future)
4. **Diagnostics** - System status

### Button Controls

- **Mode Button (GPIO4)**: Cycle through modes
- **Function Button (GPIO5)**: Mode-specific actions
- **Long press (>2s)**: Enter settings

## Development Status

- [x] Project structure and pinout documentation
- [x] Custom JHD1313M3 LCD driver
- [x] I2C bus initialization
- [x] Button input handling
- [x] RGB backlight color control
- [ ] MPU6050 sensor integration
- [ ] Angle calculation and filtering
- [ ] Calibration routine
- [ ] Mode switching logic
- [ ] NVS storage for calibration data
- [ ] Low power sleep mode

## License

MIT License - See LICENSE file for details

## Author

Created for camper levelling applications.
