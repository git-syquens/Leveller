# Quick Build Guide - Clean Build

## Problem
The error `lvgl_esp32_drivers` indicates old incompatible components were cached.

## Solution: Clean Build

### Step 1: Clean Everything
```bash
# In your project directory (E:\Dev\Leveller)

# Delete build artifacts
rm -rf build

# Delete managed components (force fresh download)
rm -rf managed_components

# Delete dependency lock
rm dependencies.lock

# Optional: Delete sdkconfig (will regenerate from sdkconfig.defaults)
rm sdkconfig
```

### Step 2: Set Target
```bash
idf.py set-target esp32
```

### Step 3: Build
```bash
idf.py build
```

This will:
1. Download fresh components from ESP Component Registry:
   - `espressif/esp_lvgl_port` v2.6.2+
   - `atanisoft/esp_lcd_touch_xpt2046` v1.0.6+
   - `lvgl/lvgl` v9.2.0+ (explicit, prevents old drivers)
2. Create `dependencies.lock` with correct versions
3. Build the project

### Step 4: Flash
```bash
idf.py -p COM3 flash monitor
```

## If Build Still Fails

### Check for Old Components
```bash
# Look for old driver remnants
find managed_components -name "*lvgl_esp32_drivers*"

# If found, delete them:
rm -rf managed_components/lvgl__lvgl_esp32_drivers
```

### Force Component Update
```bash
idf.py fullclean
idf.py reconfigure
idf.py build
```

## Verify Correct Components

After successful build, check:
```bash
cat dependencies.lock
```

Should show:
- `espressif/esp_lvgl_port`: 2.6.x
- `atanisoft/esp_lcd_touch_xpt2046`: 1.0.6
- `lvgl/lvgl`: 9.2.x or 9.3.x
- **NO** `lvgl/lvgl_esp32_drivers` (old, incompatible)

## Expected Output

```
-- Building ESP-IDF components for target esp32
-- Checking Python dependencies...
Processing 3 dependencies:
[1/3] espressif/esp_lvgl_port (2.6.2)
[2/3] atanisoft/esp_lcd_touch_xpt2046 (1.0.6)
[3/3] lvgl/lvgl (9.3.0)
-- Project sdkconfig file E:/Dev/Leveller/sdkconfig
...
[100%] Built target leveller.elf
```

## Common Issues

### Issue: "driver/spi_master.h not found"
**Cause**: Old `lvgl_esp32_drivers` component being pulled in
**Fix**: Delete `managed_components` and rebuild

### Issue: LVGL API errors (lv_obj_t, lv_display_t, etc.)
**Cause**: Mixing LVGL 8.x and 9.x APIs
**Fix**: Ensure `dependencies.lock` has LVGL 9.x

### Issue: Component version conflicts
**Cause**: Cached dependencies with wrong versions
**Fix**: Delete `dependencies.lock` and rebuild

## Build from VSCode

If using VSCode with ESP-IDF extension:

1. **Clean**: Ctrl+Shift+P → "ESP-IDF: Full Clean"
2. **Set Target**: Ctrl+Shift+P → "ESP-IDF: Set Espressif Device Target"
3. **Build**: Ctrl+Shift+P → "ESP-IDF: Build Project"
4. **Flash**: Ctrl+Shift+P → "ESP-IDF: Flash Device"
5. **Monitor**: Ctrl+Shift+P → "ESP-IDF: Monitor Device"

Or use the bottom toolbar buttons.

## Success Indicators

✅ Build completes without errors
✅ `managed_components/lvgl__lvgl/` exists (v9.x)
✅ `managed_components/espressif__esp_lvgl_port/` exists
✅ `managed_components/atanisoft__esp_lcd_touch_xpt2046/` exists
✅ NO `lvgl_esp32_drivers` directory
✅ Flash size shows: ~1.5MB binary

## Next Steps After Successful Build

```bash
# Flash and monitor
idf.py -p COM3 flash monitor

# Expected serial output:
# I (123) LEVELLER: === Leveller - High-Performance Demo ===
# I (456) LEVELLER: LCD initialized successfully (320x240 landscape)
# I (789) LEVELLER: LVGL initialized successfully
# I (1000) LEVELLER: FPS: 58.43
```

You should see:
- White screen with black "Hello World" text moving
- Touch top → blue for 1 second
- Touch bottom → red for 1 second
- FPS counter in serial output

---

**TL;DR**: Delete `build/`, `managed_components/`, and `dependencies.lock`, then rebuild.
