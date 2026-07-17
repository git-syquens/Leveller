# Display Troubleshooting Session - 2025-12-19

## Initial Success
Display was working initially with:
- Moving "Hello World" sprite across screen
- FPS counter showing in serial
- BUT: Display was **distorted** and **not in proper landscape mode** (text moved mostly across upper part)

## Problem Encountered
After working briefly, the display:
1. Turned completely **white**
2. **Stayed white permanently** - even after:
   - Power cycles
   - Reflashing firmware
   - Code changes
   - NVS erase attempts

## Troubleshooting Steps Taken

### 1. Simplified Code (No Effect)
- Removed all animation code
- Removed FPS tracking
- Removed touch handlers
- Removed all tasks
- Created simple static "Hello World" display
- **Result**: Still white screen

### 2. Tried Different Colors/Styles (No Effect)
- Changed background from white to red
- Changed text from black to white
- Added opacity settings (`LV_OPA_COVER`, `LV_OPA_TRANSP`)
- Removed all default styles with `lv_obj_remove_style_all()`
- Used explicit `LV_PART_MAIN` selectors
- **Result**: Still white screen

### 3. Created Test Rectangle (No Effect)
- Drew 200x100 red rectangle with blue border
- Added "TEST" label
- Forced screen refresh with `lv_refr_now(NULL)`
- **Result**: Still white screen

### 4. Direct LCD Hardware Test (PENDING)
Added direct LCD panel drawing to bypass LVGL completely:
```c
// Draw red bar at top (10px)
esp_lcd_panel_draw_bitmap(lcd_panel, 0, 0, 320, 10, red_buffer);

// Draw blue bar at bottom (10px)
esp_lcd_panel_draw_bitmap(lcd_panel, 0, 230, 320, 240, blue_buffer);
```
**Status**: Code added but user reports still white screen

### 5. NVS Erase Attempt (No Effect)
Forced NVS erase on every boot:
```c
ESP_ERROR_CHECK(nvs_flash_erase());
esp_err_t ret = nvs_flash_init();
```
**Result**: Still white screen

## Observations

### What Works
- ✅ Display backlight turns on (GPIO21)
- ✅ Software initializes without errors
- ✅ Serial output shows all initialization steps complete
- ✅ LVGL initializes successfully
- ✅ Display showed content ONCE (the distorted moving text)

### What Doesn't Work
- ❌ LVGL rendering (only shows white)
- ❌ Direct LCD panel drawing (still shows white - **THIS IS CRITICAL**)
- ❌ Any color changes persist
- ❌ Screen stays frozen in white state

## Critical Finding
**The direct LCD hardware test showing white screen is the smoking gun.**

If `esp_lcd_panel_draw_bitmap()` doesn't show the red/blue bars, then either:
1. The LCD panel is locked in a strange state
2. The display controller registers are set incorrectly
3. There's a hardware issue (but unlikely since it worked once)
4. The ILI9341 driver initialization is leaving the display in wrong mode

## Hardware Configuration (VERIFIED WORKING)
```c
// Correct pins for ALI Purple Screen Board
#define LCD_PIN_MISO        12
#define LCD_PIN_MOSI        13
#define LCD_PIN_CLK         14
#define LCD_PIN_CS          15
#define LCD_PIN_DC          2
#define LCD_PIN_RST         -1  // Not used
#define LCD_PIN_BCKL        21  // Backlight
```

## Current Display Settings
```c
// Panel initialization
esp_lcd_panel_reset(lcd_panel);
esp_lcd_panel_init(lcd_panel);
esp_lcd_panel_invert_color(lcd_panel, true);
esp_lcd_panel_swap_xy(lcd_panel, true);     // Landscape mode
esp_lcd_panel_mirror(lcd_panel, false, true);
esp_lcd_panel_disp_on_off(lcd_panel, true);

// RGB order
.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR

// LVGL rotation (applied AFTER panel swap_xy)
.rotation = {
    .swap_xy = false,
    .mirror_x = false,
    .mirror_y = false,
}
```

## Theories to Investigate Tomorrow

### Theory 1: Display Controller State Corruption
The ILI9341 might be stuck in a bad state. Try:
- Remove `esp_lcd_panel_invert_color(lcd_panel, true)` - try false or remove entirely
- Try different `rgb_ele_order` (RGB instead of BGR)
- Add explicit ILI9341 register writes to force known-good state
- Send raw SPI commands to ILI9341 to reset it properly

### Theory 2: Orientation/Window Settings Wrong
The display worked but was "distorted" - suggests window/orientation mismatch. Try:
- Remove `esp_lcd_panel_swap_xy()` completely
- Try different mirror combinations
- Check if LVGL's `.rotation.swap_xy` conflicts with panel's `swap_xy`
- Verify the display resolution is actually 320x240 (might be 240x320)

### Theory 3: Memory/DMA Issue
Direct drawing uses DMA buffers. Try:
- Verify `MALLOC_CAP_DMA` is working correctly
- Try smaller buffer (single line instead of 10 lines)
- Check if SPI2_HOST is properly configured for DMA
- Verify `max_transfer_sz` is correct

### Theory 4: Display is Actually Rendering (But Wrong Area)
The white might be LVGL's default background successfully rendering. Try:
- Use completely different approach: draw raw pixels to known coordinates
- Send ILI9341 column/page address commands manually
- Verify the drawing window is set to full screen (0,0 to 319,239)

### Theory 5: Timing/Sequence Issue
The display worked briefly, then froze. Try:
- Add delays between initialization steps
- Slow down SPI clock from 40MHz to 20MHz or 10MHz
- Check if `trans_queue_depth` of 10 is causing buffer issues
- Verify double buffering isn't causing race conditions

## Next Steps (Priority Order)

### 1. IMMEDIATE: Test Hardware Drawing
Verify if the direct LCD hardware test actually ran:
- Check serial output for "Direct LCD test complete" message
- If message appears but no red/blue bars visible → **hardware drawing broken**
- If message doesn't appear → code didn't execute (check for crash before this point)

### 2. Try Minimal ILI9341 Init
Strip down to absolute minimum:
```c
esp_lcd_panel_reset(lcd_panel);
esp_lcd_panel_init(lcd_panel);
esp_lcd_panel_disp_on_off(lcd_panel, true);
// Skip: invert_color, swap_xy, mirror
// Draw directly to panel - should see something
```

### 3. Send Raw ILI9341 Commands
Bypass the esp_lcd driver completely and send raw register writes:
```c
// Set to RGB mode
uint8_t data = 0x08;  // MY=0, MX=0, MV=0, ML=0, BGR=1, MH=0
esp_lcd_panel_io_tx_param(lcd_io, 0x36, &data, 1);  // MADCTL

// Write pixel data directly
esp_lcd_panel_io_tx_param(lcd_io, 0x2C, pixel_data, size);  // RAMWR
```

### 4. Compare with Working GitHub Code
The original working code used old-style drivers. Key differences to investigate:
- Different SPI configuration
- Different initialization sequence
- Different LVGL integration method (no esp_lvgl_port)

### 5. Nuclear Option: Use Old Driver Method
As last resort, abandon esp_lvgl_port and use the exact method from the working GitHub code:
- Direct SPI writes
- Manual LVGL integration
- Pixel-by-pixel drawing if needed

## Files Modified This Session
- [main/main.c](main/main.c) - Simplified, added tests, forced NVS erase
- [LEVELLER_PINOUT.md](LEVELLER_PINOUT.md) - Corrected pins for ALI Purple Screen Board
- [DISPLAY_PINOUT_CORRECTED.md](DISPLAY_PINOUT_CORRECTED.md) - Documented pin correction

## Code State
**Current main.c state:**
- Simple static UI (no animation)
- Direct LCD hardware test (red/blue bars)
- NVS forced erase every boot
- Test rectangle + label (not showing)

## Key Questions for Tomorrow
1. Did the direct LCD hardware test message appear in serial?
2. Is there ANY visible change when flashing (flicker, color flash, anything)?
3. Does the backlight brightness change or is it always the same?
4. Can we power cycle JUST the display (not the ESP32) to reset it?
5. Is there a way to verify the ILI9341 is responding to commands (read RDDID register)?

## Hypothesis to Test First Tomorrow
**The display controller might need a proper reset sequence that we're not providing.**

Since `LCD_PIN_RST = -1` (not connected), the ILI9341 might not be properly resetting. The working GitHub code also has RST=-1, but:
- The first flash might have properly reset it (power on reset)
- The distorted display suggests partial initialization
- The freeze to white suggests the controller entered sleep/power-save mode
- Without a hardware reset line, we can't force it out of that state

**Test**: Try adding a software reset command:
```c
uint8_t reset_cmd = 0x01;  // SWRESET
esp_lcd_panel_io_tx_param(lcd_io, reset_cmd, NULL, 0);
vTaskDelay(pdMS_TO_TICKS(120));  // Wait 120ms after reset
```

---

## Summary
Display worked once (distorted), then froze to white permanently. Even direct hardware drawing shows white. Suspicion: ILI9341 display controller is stuck in wrong mode and needs proper reset/re-initialization sequence. Hardware and pins are verified correct. LVGL and esp_lcd drivers are initializing without errors but not producing visible output.
