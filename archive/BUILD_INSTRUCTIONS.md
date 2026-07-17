# Leveller - Build Instructions

## Project Overview

ESP-IDF project for a camper levelling indicator using:
- **Display**: ILI9341 320x240 TFT LCD (SPI)
- **Touch**: XPT2046 resistive touch controller (SPI)
- **Graphics**: LVGL 9.x with esp_lvgl_port
- **MCU**: ESP32-WROOM-32 (ESP32-SCREEN board)

---

## Prerequisites

### 1. ESP-IDF Installation
```bash
# ESP-IDF v5.0 or later required
idf.py --version
```

### 2. Component Dependencies (Auto-installed)
The following components will be automatically downloaded by IDF Component Manager:
- `espressif/esp_lvgl_port` v2.6.2+
- `atanisoft/esp_lcd_touch_xpt2046` v1.0.6+
- `lvgl/lvgl` v9.x (dependency of esp_lvgl_port)

---

## Build Process

### 1. Set ESP-IDF Target
```bash
idf.py set-target esp32
```

### 2. Configure Project (Optional)
```bash
idf.py menuconfig
```

Navigate to `Leveller Configuration` to adjust:
- LCD resolution and SPI clock speed
- Touch controller settings
- Pin assignments
- I2C configuration (for future MPU6050 integration)

### 3. Build Project
```bash
idf.py build
```

This will:
- Download all component dependencies to `managed_components/`
- Generate `dependencies.lock` file
- Compile the project

### 4. Flash to ESP32
```bash
idf.py -p COM3 flash
```

Replace `COM3` with your actual serial port (e.g., `/dev/ttyUSB0` on Linux).

### 5. Monitor Serial Output
```bash
idf.py -p COM3 monitor
```

Or combine flash + monitor:
```bash
idf.py -p COM3 flash monitor
```

Exit monitor: `Ctrl+]`

---

## Project Structure

```
Leveller/
├── CMakeLists.txt              # Top-level build configuration
├── sdkconfig.defaults          # Default ESP32 configuration
├── main/
│   ├── CMakeLists.txt          # Main component build rules
│   ├── idf_component.yml       # Component dependencies
│   ├── Kconfig.projbuild       # Project configuration menu
│   └── main.c                  # Application entry point
├── managed_components/         # Auto-downloaded dependencies (gitignored)
├── build/                      # Build output (gitignored)
├── dependencies.lock           # Locked dependency versions
└── *.md                        # Documentation files
```

---

## Pin Configuration

### Default Pins (from LEVELLER_PINOUT.md)

**TFT LCD (SPI2):**
- MOSI: GPIO12
- CLK: GPIO2
- CS: GPIO13
- DC: GPIO15
- RST: GPIO14

**Touch (SPI3):**
- MOSI: GPIO32
- MISO: GPIO36
- CLK: GPIO25
- CS: GPIO33
- IRQ: GPIO39

**I2C (MPU6050 - future):**
- SDA: GPIO21
- SCL: GPIO22

All pins can be reconfigured via `idf.py menuconfig`.

---

## Troubleshooting

### Build Errors

**"Component not found"**
```bash
# Clean and rebuild to re-download components
rm -rf managed_components dependencies.lock
idf.py build
```

**"SPI bus already initialized"**
- Check that LCD and Touch use different SPI hosts (SPI2 and SPI3)

### Display Issues

**Black screen:**
1. Check backlight GPIO (currently not used, display has built-in backlight)
2. Verify SPI pins match hardware
3. Check `esp_lcd_panel_disp_on_off(lcd_panel, true)` is called

**Garbage/artifacts:**
1. Reduce SPI clock: `idf.py menuconfig` → Leveller Configuration → Display → LCD SPI Clock
2. Try 26MHz or 20MHz instead of 40MHz
3. Check wiring quality

**Inverted colors:**
- Adjust `esp_lcd_panel_invert_color()` in main.c

### Touch Issues

**Touch not responding:**
1. Verify IRQ pin (GPIO39) is input-only capable
2. Check touch calibration in TOUCHSCREEN.md
3. Enable touch debug logs in menuconfig

### Memory Issues

**Heap allocation failed:**
- LVGL buffers use DMA-capable RAM
- Reduce `LVGL_BUFFER_HEIGHT` from 40 to 20 lines
- Check `esp_get_free_heap_size()` output

---

## Performance

### Expected Results
- **Full screen clear**: < 20ms
- **Dial needle update**: 5-10ms
- **Touch response**: < 50ms
- **FPS**: 30-60 (depends on complexity)

### Optimization Tips
1. Use `CONFIG_COMPILER_OPTIMIZATION_PERF=y` (already in sdkconfig.defaults)
2. Enable SPI ISR in IRAM: `CONFIG_SPI_MASTER_ISR_IN_IRAM=y`
3. Use LVGL dirty rectangle optimization (automatic)
4. Increase LVGL buffer size for smoother rendering

---

## Next Steps

### Immediate
- [x] Basic project structure
- [x] LCD driver integration
- [x] Touch controller integration
- [x] LVGL initialization
- [x] Test UI rendering

### Phase 2 - Sensors
- [ ] Add MPU6050 I2C driver
- [ ] Add GPS UART driver
- [ ] Implement sensor data fusion

### Phase 3 - UI Development
- [ ] Create levelling indicator UI (bubble level graphic)
- [ ] Add pitch/roll angle display
- [ ] GPS heading and position display
- [ ] Calibration screen

### Phase 4 - Features
- [ ] NVS storage for calibration data
- [ ] Audio feedback (speaker on GPIO26)
- [ ] RGB LED status indicator
- [ ] Low-power sleep mode

---

## References

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [LVGL Documentation](https://docs.lvgl.io/)
- [esp_lvgl_port Component](https://components.espressif.com/components/espressif/esp_lvgl_port)
- [XPT2046 Touch Driver](https://components.espressif.com/components/atanisoft/esp_lcd_touch_xpt2046)
- Project Documentation:
  - [LEVELLER_PINOUT.md](LEVELLER_PINOUT.md) - Complete hardware reference
  - [LVGL_PERFORMANCE_GUIDE.md](LVGL_PERFORMANCE_GUIDE.md) - Optimization guide
  - [TOUCHSCREEN.md](TOUCHSCREEN.md) - Touch calibration

---

## Support

For issues:
1. Check serial monitor output: `idf.py monitor`
2. Enable verbose logging: `idf.py menuconfig` → Component config → Log output → Verbose
3. Review pin assignments in menuconfig
4. Verify hardware connections against LEVELLER_PINOUT.md
