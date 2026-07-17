# ESP32-SCREEN Touchscreen Programming Guide
**Board**: ESP32-SCREEN (Marauder) with ESP32-WROOM-32
**Display**: 2.4" ILI9341 TFT LCD (320x240)
**Touch**: XPT2046 Resistive Touch Controller
**Last Updated**: 2025-12-19

---

## Table of Contents
1. [Hardware Overview](#hardware-overview)
2. [Pin Configuration](#pin-configuration)
3. [Component Architecture](#component-architecture)
4. [TFT LCD Programming](#tft-lcd-programming)
5. [Touch Controller Programming](#touch-controller-programming)
6. [Touch Calibration](#touch-calibration)
7. [Complete Working Example](#complete-working-example)
8. [Performance Optimization](#performance-optimization)
9. [Common Issues and Solutions](#common-issues-and-solutions)

---

## Hardware Overview

### Display Specifications
- **Type**: 2.4" TFT LCD
- **Controller**: ILI9341
- **Resolution**: 320x240 pixels (landscape) / 240x320 pixels (portrait)
- **Color Depth**: 16-bit RGB565 (65,536 colors)
- **Interface**: SPI (30 MHz clock speed)
- **Backlight**: GPIO-controlled, always-on via PIN_NUM_BCKL

### Touch Controller Specifications
- **Type**: XPT2046 4-wire resistive touch
- **Interface**: SPI (separate from LCD)
- **Interrupt**: GPIO39 (active low when screen touched)
- **ADC Range**: ~400-3700 (raw coordinates)
- **Calibration**: Required for accurate coordinate mapping

---

## Pin Configuration

### TFT LCD Pins (SPI Bus 1)

| Function | ESP32 Pin | Header Define | Description |
|----------|-----------|---------------|-------------|
| MOSI (SDI) | GPIO12 | PIN_NUM_MOSI | SPI data to display |
| MISO (SDO) | Not connected | PIN_NUM_MISO | Not used (display write-only) |
| CLK (SCK) | GPIO14 | PIN_NUM_CLK | SPI clock (30 MHz) |
| CS | GPIO15 | PIN_NUM_CS | Chip select (active low) |
| DC (RS) | GPIO2 | PIN_NUM_DC | Data/Command select (0=cmd, 1=data) |
| RST | Not connected | PIN_NUM_RST | Hardware reset (not used) |
| Backlight | GPIO21 | PIN_NUM_BCKL | Backlight control (set HIGH) |

**Important**: The header file `lcd.h` has **INCORRECT** pin definitions:
```c
// WRONG VALUES IN lcd.h (do not use):
#define PIN_NUM_MISO	12  // Should be unused
#define PIN_NUM_MOSI	13  // WRONG - should be 12
#define PIN_NUM_CLK		14  // Correct
#define PIN_NUM_CS		15  // Correct
#define PIN_NUM_DC		2   // Correct
#define PIN_NUM_RST		-1  // Correct (unused)
#define PIN_NUM_BCKL	21  // Correct
```

**ACTUAL PINS** (per schematic ESP32_CREEN.pdf):
```c
TFT_RST:  IO14 (not connected in driver)
TFT_SCK:  IO2  -> Defined as PIN_NUM_DC (WRONG!)
TFT_RS:   IO15 -> Should be DC
TFT_CS:   IO13
TFT_SDI:  IO12 -> MOSI
```

**Critical**: Despite the incorrect #defines, the driver actually works because the SPI initialization uses the correct pins. The defines are misleading but don't affect functionality.

### Touch Controller Pins (Separate SPI Bus)

| Function | ESP32 Pin | Header Define | Description |
|----------|-----------|---------------|-------------|
| MOSI (DIN) | GPIO32 | XPT2046_MOSI | SPI data to touch controller |
| MISO (DOUT) | GPIO39 | XPT2046_MISO | SPI data from touch controller |
| CLK | GPIO25 | XPT2046_CLK | SPI clock (2 MHz) |
| CS | GPIO33 | XPT2046_CS | Chip select (active low) |
| IRQ | GPIO36 | XPT2046_IRQ | Interrupt (active low on touch) |

**Note**: Touch uses a **separate SPI bus** from the LCD. GPIO39 serves dual purpose: MISO for touch data AND interrupt input.

---

## Component Architecture

### Directory Structure
```
Leveller/
├── boards/marauder/components/
│   ├── LCD/                    # TFT display driver
│   │   ├── include/lcd.h       # LCD API and color definitions
│   │   ├── lcd.c               # ILI9341 driver implementation
│   │   └── CMakeLists.txt
│   ├── GUI/                    # Graphics primitives
│   │   ├── include/gui.h       # Drawing functions API
│   │   ├── include/font.h      # Font data
│   │   ├── gui.c               # Shapes, text, graphics
│   │   └── CMakeLists.txt
│   └── XPT2046/                # Touch controller driver
│       ├── include/xpt2046.h   # Touch API
│       ├── xpt2046.c           # Touch driver implementation
│       └── CMakeLists.txt
└── main/
    ├── main.c                  # Your application code
    └── CMakeLists.txt
```

### CMakeLists.txt Configuration

**Root CMakeLists.txt** (`Leveller/CMakeLists.txt`):
```cmake
cmake_minimum_required(VERSION 3.16)

# Add Marauder board components
set(EXTRA_COMPONENT_DIRS "boards/marauder/components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(leveller)
```

**Main Component CMakeLists.txt** (`main/CMakeLists.txt`):
```cmake
idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS ""
                    REQUIRES driver LCD GUI XPT2046)
```

### Required Includes
```c
#include "lcd.h"        // TFT display functions
#include "gui.h"        // Graphics/drawing functions
#include "xpt2046.h"    // Touch controller functions
```

---

## TFT LCD Programming

### Color Definitions (RGB565 Format)

All colors are 16-bit RGB565 format (5 bits red, 6 bits green, 5 bits blue):

```c
// Basic Colors
#define WHITE       0xFFFF
#define BLACK       0x0000
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define YELLOW      0xFFE0
#define CYAN        0x7FFF
#define MAGENTA     0xF81F

// Extended Colors
#define BROWN       0xBC40
#define GRAY        0x8430
#define DARKBLUE    0x01CF
#define LIGHTBLUE   0x7D7C
#define LIGHTGREEN  0x841F
#define LIGHTGRAY   0xEF5B
```

### Core LCD Functions

#### Initialization
```c
/**
 * @brief Initialize LCD display
 * @param color Initial background color (RGB565)
 * @note This function:
 *       - Initializes SPI bus (30 MHz)
 *       - Sends ILI9341 initialization sequence
 *       - Sets backlight ON (GPIO21 HIGH)
 *       - Clears screen to specified color
 *       - Sets default orientation to PORTRAIT
 */
void Init_LCD(uint16_t color);

// Example:
Init_LCD(WHITE);  // Initialize with white background
```

#### Orientation Control
```c
/**
 * @brief Set screen orientation
 * @param orientation One of:
 *        - LCD_DISPLAY_ORIENTATION_PORTRAIT (0)          - 240x320, default
 *        - LCD_DISPLAY_ORIENTATION_PORTRAIT_INVERTED (1) - 240x320, flipped 180°
 *        - LCD_DISPLAY_ORIENTATION_LANDSCAPE (2)         - 320x240, rotated 90° CW
 *        - LCD_DISPLAY_ORIENTATION_LANDSCAPE_INVERTED (3)- 320x240, rotated 90° CCW
 * @note Updates global variables LCD_Width and LCD_Height
 */
void LCD_Set_Orientation(uint8_t orientation);

// Example:
LCD_Set_Orientation(LCD_DISPLAY_ORIENTATION_LANDSCAPE);
// After this: LCD_Width = 320, LCD_Height = 240
```

#### Screen Clearing
```c
/**
 * @brief Clear entire screen to a color
 * @param Color RGB565 color value
 * @warning SLOW! Writes each pixel individually (76,800 pixels)
 *          Takes ~2-3 seconds to complete
 * @note Use LCD_DrawFillRectangle() instead if clearing after init
 */
void LCD_Clear(uint16_t Color);

// Example:
LCD_Clear(WHITE);  // SLOW - avoid calling repeatedly
```

#### Low-Level Drawing
```c
/**
 * @brief Send command to LCD controller
 * @param cmd ILI9341 command byte
 */
void LCD_WriteCMD(const uint8_t cmd);

/**
 * @brief Send data to LCD controller
 * @param data Pointer to data buffer
 * @param len Number of bytes to send
 */
void LCD_WriteDate(const uint8_t *data, int len);

/**
 * @brief Send 16-bit data word to LCD
 * @param data 16-bit data (e.g., RGB565 color)
 */
void LCD_WriteDate16(uint16_t data);

/**
 * @brief Set drawing window (clipping region)
 * @param xStar Starting X coordinate
 * @param yStar Starting Y coordinate
 * @param xEnd Ending X coordinate
 * @param yEnd Ending Y coordinate
 * @note Subsequent writes only affect this region
 */
void LCD_SetWindows(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd);
```

### Graphics Functions (from gui.h)

#### Points and Pixels
```c
/**
 * @brief Draw a single pixel
 * @param x X coordinate
 * @param y Y coordinate
 * @param color RGB565 color
 */
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
```

#### Lines
```c
/**
 * @brief Draw a line (1 pixel wide)
 * @param x1, y1 Start coordinates
 * @param x2, y2 End coordinates
 * @param color RGB565 color
 */
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

/**
 * @brief Draw a thick line
 * @param x1, y1 Start coordinates
 * @param x2, y2 End coordinates
 * @param size Line thickness (0-2)
 * @param color RGB565 color
 */
void LCD_DrawBLine1(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                    uint8_t size, uint16_t color);
```

#### Shapes
```c
/**
 * @brief Draw rectangle outline
 * @param x1, y1 Top-left corner
 * @param x2, y2 Bottom-right corner
 * @param color RGB565 color
 */
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                       uint16_t color);

/**
 * @brief Draw filled rectangle (FASTER than LCD_Clear for regions)
 * @param sx, sy Top-left corner
 * @param ex, ey Bottom-right corner
 * @param color RGB565 color
 * @note More efficient than LCD_Clear for partial screen updates
 */
void LCD_DrawFillRectangle(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey,
                           uint16_t color);

/**
 * @brief Draw circle outline
 * @param x0, y0 Center coordinates
 * @param r Radius in pixels
 * @param color RGB565 color
 */
void LCD_Draw_Circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);

/**
 * @brief Draw filled circle
 * @param x0, y0 Center coordinates
 * @param r Radius in pixels
 * @param color RGB565 color
 */
void LCD_Draw_FillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
```

#### Text Display
```c
/**
 * @brief Display a string
 * @param x X coordinate (top-left of first character)
 * @param y Y coordinate (top-left of first character)
 * @param bcolor Background color (RGB565)
 * @param fcolor Foreground/text color (RGB565)
 * @param size Font size: 12, 16, 24, or 32 pixels
 * @param p Null-terminated string to display
 * @param mode 0=opaque background, 1=transparent background
 */
void LCD_ShowString(uint16_t x, uint16_t y, uint16_t bcolor, uint16_t fcolor,
                   uint8_t size, char *p, uint8_t mode);

// Examples:
LCD_ShowString(10, 10, WHITE, BLACK, 24, "Hello World", 0);  // Opaque
LCD_ShowString(10, 50, 0, RED, 16, "Status: OK", 1);        // Transparent

/**
 * @brief Display a single character
 * @param x, y Top-left position
 * @param bcolor Background color
 * @param fcolor Text color
 * @param ch Character to display (ASCII)
 * @param size Font size (12, 16, 24, 32)
 * @param mode 0=opaque, 1=transparent
 */
void LCD_ShowChar(uint16_t x, uint16_t y, uint16_t bcolor, uint16_t fcolor,
                 uint8_t ch, uint8_t size, uint8_t mode);

/**
 * @brief Display a number
 * @param x, y Position
 * @param bcolor Background color
 * @param fcolor Text color
 * @param num Number to display
 * @param len Number of digits (zero-padded)
 * @param size Font size
 */
void LCD_ShowNum(uint16_t x, uint16_t y, uint16_t bcolor, uint16_t fcolor,
                uint32_t num, uint8_t len, uint8_t size);
```

### Font Sizes and Spacing

| Size | Width | Height | Character Spacing |
|------|-------|--------|-------------------|
| 12 | 6px | 12px | ~8px total width |
| 16 | 8px | 16px | ~10px total width |
| 24 | 12px | 24px | ~14px total width |
| 32 | 16px | 32px | ~18px total width |

**Text Positioning Examples** (Landscape 320x240):
```c
// Centered text (size 24, ~11 chars "Hello World")
LCD_ShowString(100, 110, WHITE, BLACK, 24, "Hello World", 0);

// Top-left corner
LCD_ShowString(5, 5, WHITE, RED, 16, "Status", 0);

// Bottom-right aligned (approx)
LCD_ShowString(250, 220, WHITE, BLUE, 12, "Info", 0);
```

---

## Touch Controller Programming

### Global Variables
```c
extern uint16_t TouchX;  // Calibrated X coordinate (0-LCD_Width)
extern uint16_t TouchY;  // Calibrated Y coordinate (0-LCD_Height)

extern float xfac;       // X calibration factor
extern float yfac;       // Y calibration factor
extern short xoff;       // X offset
extern short yoff;       // Y offset
```

### Core Touch Functions

#### Initialization
```c
/**
 * @brief Initialize XPT2046 touch controller
 * @note Configures GPIO36 as IRQ input
 *       Sets up SPI communication pins
 *       Does NOT initialize calibration values
 */
void xpt2046_init(void);

// Example:
xpt2046_init();
```

#### Touch Reading
```c
/**
 * @brief Read touch coordinates
 * @return true if screen is currently touched, false otherwise
 * @note When returns true:
 *       - TouchX and TouchY are updated with screen coordinates
 *       - Coordinates are calibrated using xfac/yfac/xoff/yoff
 *       - IRQ pin (GPIO36) must be LOW (touch detected)
 * @warning Requires valid calibration values to be set first!
 */
bool xpt2046_read(void);

// Example:
if (xpt2046_read()) {
    printf("Touch at X=%d, Y=%d\n", TouchX, TouchY);
}
```

#### Low-Level Functions
```c
/**
 * @brief Read raw ADC value for X or Y axis
 * @param xy Command byte: CMD_X_READ or CMD_Y_READ
 * @return Raw ADC value (~400-3700 range)
 */
uint16_t TP_Read_XOY(uint8_t xy);

/**
 * @brief Full-screen touch calibration routine
 * @note Interactive calibration:
 *       - Displays 4 calibration points
 *       - User must touch each point
 *       - Calculates xfac/yfac/xoff/yoff automatically
 *       - Takes ~30-60 seconds
 */
void TP_Adjust(void);
```

---

## Touch Calibration

### Why Calibration is Required

The XPT2046 returns raw ADC values (~400-3700), which must be converted to screen coordinates (0-320 or 0-240) using:

```c
screen_x = xfac * raw_adc_x + xoff;
screen_y = yfac * raw_adc_y + yoff;
```

Without calibration, `xfac`/`yfac`/`xoff`/`yoff` are **uninitialized (0)**, causing incorrect coordinates.

### Method 1: Manual Calibration (Recommended for Quick Testing)

Use reference values from the header file comments:

```c
// Reference calibration points (from xpt2046.h):
// Point 1 (top-left):     ADX=3672, ADY=513
// Point 2 (top-right):    ADX=3691, ADY=3600
// Point 3 (bottom-left):  ADX=578,  ADY=418
// Point 4 (bottom-right): ADX=540,  ADY=3612

// Calculate calibration factors:
// xfac = (screen_width - 40) / (point1_ADX - point3_ADX)
// yfac = (screen_height - 40) / (point4_ADY - point3_ADY)
// xoff = (screen_width - xfac * (point1_ADX + point3_ADX)) / 2
// yoff = (screen_height - yfac * (point4_ADY + point3_ADY)) / 2

// For landscape mode (320x240):
xfac = 0.0905f;  // (320-40) / (3672-578)
yfac = 0.0626f;  // (240-40) / (3612-418)
xoff = (short)((320.0f - xfac * (3672.0f + 578.0f)) / 2.0f);
yoff = (short)((240.0f - yfac * (3612.0f + 418.0f)) / 2.0f);
```

**Complete Calibration Code**:
```c
void setup_touch_calibration(void) {
    // Initialize touch hardware
    xpt2046_init();

    // Set manual calibration values for landscape mode (320x240)
    xfac = 0.0905f;
    yfac = 0.0626f;
    xoff = (short)((320.0f - xfac * (3672.0f + 578.0f)) / 2.0f);
    yoff = (short)((240.0f - yfac * (3612.0f + 418.0f)) / 2.0f);

    ESP_LOGI("TOUCH", "Calibration: xfac=%.4f, yfac=%.4f, xoff=%d, yoff=%d",
             xfac, yfac, xoff, yoff);
}
```

### Method 2: Interactive Calibration (Accurate but Time-Consuming)

```c
void do_full_calibration(void) {
    xpt2046_init();

    // Full interactive calibration (requires user to touch 4 points)
    TP_Adjust();

    // Calibration values now automatically set
    ESP_LOGI("TOUCH", "Calibration complete: xfac=%.4f, yfac=%.4f, xoff=%d, yoff=%d",
             xfac, yfac, xoff, yoff);
}
```

### Calibration for Different Orientations

Calibration values are **orientation-dependent**. Reference values are for landscape mode:

| Orientation | Width x Height | Use Reference Values? |
|-------------|----------------|------------------------|
| Portrait (0) | 240x320 | No - need recalibration |
| Portrait Inverted (1) | 240x320 | No - need recalibration |
| **Landscape (2)** | **320x240** | **Yes - use reference** |
| Landscape Inverted (3) | 320x240 | Maybe - test first |

**Important**: If changing orientation, you must recalibrate or adjust `xfac`/`yfac`/`xoff`/`yoff` accordingly.

### Verifying Calibration

```c
void test_calibration(void) {
    ESP_LOGI("TEST", "Touch screen corners to verify calibration:");

    while (1) {
        if (xpt2046_read()) {
            ESP_LOGI("TEST", "Touch: X=%d, Y=%d", TouchX, TouchY);

            // Landscape mode (320x240)
            if (TouchX < 10 && TouchY < 10) {
                ESP_LOGI("TEST", "Top-left corner detected!");
            }
            if (TouchX > 310 && TouchY < 10) {
                ESP_LOGI("TEST", "Top-right corner detected!");
            }
            // etc...

            vTaskDelay(pdMS_TO_TICKS(500));
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

---

## Complete Working Example

### Minimal Touch + Display Application

```c
/**
 * @file main.c
 * @brief Complete touchscreen example with color-changing on touch
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lcd.h"
#include "gui.h"
#include "xpt2046.h"

static const char *TAG = "TOUCHSCREEN";

// External touch variables
extern uint16_t TouchX;
extern uint16_t TouchY;
extern float xfac;
extern float yfac;
extern short xoff;
extern short yoff;

void app_main(void)
{
    ESP_LOGI(TAG, "Touchscreen Example Starting");

    // ===== LCD INITIALIZATION =====

    // Initialize LCD (slow - clears screen pixel by pixel)
    Init_LCD(WHITE);
    ESP_LOGI(TAG, "LCD initialized");

    // Set to landscape mode (320x240)
    LCD_Set_Orientation(LCD_DISPLAY_ORIENTATION_LANDSCAPE);
    ESP_LOGI(TAG, "Orientation: LANDSCAPE (320x240)");

    // Fast clear (use this instead of LCD_Clear after init)
    LCD_DrawFillRectangle(0, 0, 319, 239, WHITE);

    // Display title text
    LCD_ShowString(80, 10, WHITE, BLACK, 24, "Touch Demo", 0);
    LCD_ShowString(50, 50, WHITE, BLUE, 16, "Touch screen to test", 1);

    // ===== TOUCH INITIALIZATION =====

    xpt2046_init();
    ESP_LOGI(TAG, "Touch controller initialized");

    // Set calibration values (landscape mode)
    xfac = 0.0905f;
    yfac = 0.0626f;
    xoff = (short)((320.0f - xfac * (3672.0f + 578.0f)) / 2.0f);
    yoff = (short)((240.0f - yfac * (3612.0f + 418.0f)) / 2.0f);

    ESP_LOGI(TAG, "Calibration: xfac=%.4f, yfac=%.4f, xoff=%d, yoff=%d",
             xfac, yfac, xoff, yoff);

    // ===== MAIN LOOP =====

    ESP_LOGI(TAG, "Touch test active:");
    ESP_LOGI(TAG, "  - Touch TOP half -> Blue");
    ESP_LOGI(TAG, "  - Touch BOTTOM half -> Red");

    while (1) {
        // Poll for touch
        if (xpt2046_read()) {
            ESP_LOGI(TAG, "Touch at X=%d, Y=%d", TouchX, TouchY);

            // Determine which half was touched (Y midpoint = 120)
            if (TouchY < 120) {
                // Top half -> Blue
                ESP_LOGI(TAG, "TOP - turning BLUE");
                LCD_DrawFillRectangle(0, 0, 319, 239, BLUE);
                vTaskDelay(pdMS_TO_TICKS(2000));

                // Restore white with text
                LCD_DrawFillRectangle(0, 0, 319, 239, WHITE);
                LCD_ShowString(80, 10, WHITE, BLACK, 24, "Touch Demo", 0);
                LCD_ShowString(50, 50, WHITE, BLUE, 16, "Touch screen to test", 1);

            } else {
                // Bottom half -> Red
                ESP_LOGI(TAG, "BOTTOM - turning RED");
                LCD_DrawFillRectangle(0, 0, 319, 239, RED);
                vTaskDelay(pdMS_TO_TICKS(2000));

                // Restore white with text
                LCD_DrawFillRectangle(0, 0, 319, 239, WHITE);
                LCD_ShowString(80, 10, WHITE, BLACK, 24, "Touch Demo", 0);
                LCD_ShowString(50, 50, WHITE, BLUE, 16, "Touch screen to test", 1);
            }
        }

        // Polling delay
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

### Advanced Touch Example: Multi-Region Detection

```c
#define REGION_TOP_LEFT      0
#define REGION_TOP_RIGHT     1
#define REGION_BOTTOM_LEFT   2
#define REGION_BOTTOM_RIGHT  3
#define REGION_CENTER        4

int get_touch_region(uint16_t x, uint16_t y) {
    // Landscape 320x240
    // Quadrants: X=160, Y=120 midpoints

    if (x >= 100 && x <= 220 && y >= 80 && y <= 160) {
        return REGION_CENTER;
    }

    if (x < 160) {
        return (y < 120) ? REGION_TOP_LEFT : REGION_BOTTOM_LEFT;
    } else {
        return (y < 120) ? REGION_TOP_RIGHT : REGION_BOTTOM_RIGHT;
    }
}

void handle_touch(void) {
    if (xpt2046_read()) {
        int region = get_touch_region(TouchX, TouchY);

        switch (region) {
            case REGION_TOP_LEFT:
                LCD_ShowString(10, 10, WHITE, RED, 16, "TL", 1);
                break;
            case REGION_TOP_RIGHT:
                LCD_ShowString(290, 10, WHITE, BLUE, 16, "TR", 1);
                break;
            case REGION_BOTTOM_LEFT:
                LCD_ShowString(10, 220, WHITE, GREEN, 16, "BL", 1);
                break;
            case REGION_BOTTOM_RIGHT:
                LCD_ShowString(290, 220, WHITE, YELLOW, 16, "BR", 1);
                break;
            case REGION_CENTER:
                LCD_DrawFillCircle(160, 120, 30, MAGENTA);
                break;
        }
    }
}
```

---

## Performance Optimization

### Issue 1: Slow Screen Clearing

**Problem**: `LCD_Clear(color)` writes each pixel individually (76,800 pixels @ 30 MHz SPI = ~2-3 seconds)

**Solution**: Use `LCD_DrawFillRectangle()` instead:
```c
// SLOW (avoid):
LCD_Clear(WHITE);

// FASTER:
LCD_DrawFillRectangle(0, 0, 319, 239, WHITE);
```

**Note**: Both functions still write pixel-by-pixel. The `Init_LCD(color)` call is unavoidable and slow, but only do it once at startup.

### Issue 2: Redundant Screen Fills

**Problem**: Calling `Init_LCD()` clears the screen, then calling `LCD_Clear()` or `LCD_DrawFillRectangle()` clears it again.

**Solution**: Remove redundant clears:
```c
// Initialize with desired color
Init_LCD(WHITE);

// Set orientation (this does NOT clear the screen)
LCD_Set_Orientation(LCD_DISPLAY_ORIENTATION_LANDSCAPE);

// NO NEED to clear again - skip this:
// LCD_DrawFillRectangle(0, 0, 319, 239, WHITE);  // Redundant!

// Just draw your content:
LCD_ShowString(100, 110, WHITE, BLACK, 24, "Hello", 0);
```

### Issue 3: Touch Polling Frequency

**Problem**: Polling `xpt2046_read()` too fast wastes CPU. Polling too slow causes missed touches.

**Solution**: Use 100ms polling interval (10 Hz):
```c
while (1) {
    if (xpt2046_read()) {
        handle_touch();
    }
    vTaskDelay(pdMS_TO_TICKS(100));  // 100ms = good balance
}
```

### Issue 4: Text Redraw Performance

**Problem**: Text rendering is relatively slow, especially large fonts.

**Solution**: Only redraw text when needed:
```c
// BAD: Redraws text every loop iteration
while (1) {
    LCD_ShowString(10, 10, WHITE, BLACK, 24, "Status", 0);  // Slow!
}

// GOOD: Draw once, update only on change
static char status[32] = "Idle";
LCD_ShowString(10, 10, WHITE, BLACK, 24, status, 0);

while (1) {
    if (status_changed) {
        LCD_DrawFillRectangle(10, 10, 200, 34, WHITE);  // Clear text area
        LCD_ShowString(10, 10, WHITE, BLACK, 24, status, 0);  // Redraw
        status_changed = false;
    }
}
```

### Optimization Summary

| Operation | Time | Frequency | Notes |
|-----------|------|-----------|-------|
| `Init_LCD()` | ~2-3s | Once at boot | Unavoidable, but only call once |
| `LCD_Clear()` | ~2-3s | Never (avoid) | Use `LCD_DrawFillRectangle()` instead |
| `LCD_DrawFillRectangle()` (full screen) | ~2-3s | Minimize | Still slow, but necessary |
| `LCD_ShowString()` (size 24) | ~100ms | On change only | Cache and update selectively |
| `xpt2046_read()` | <1ms | 10 Hz (100ms) | Fast, but don't over-poll |

---

## Common Issues and Solutions

### Issue: Touch Coordinates Always Max Value

**Symptom**: `TouchX` and `TouchY` are always 320/240 or similar max values.

**Cause**: Calibration factors `xfac`, `yfac`, `xoff`, `yoff` are not initialized (all zero).

**Solution**: Set calibration values manually:
```c
xpt2046_init();

// MUST set calibration before reading touch!
xfac = 0.0905f;
yfac = 0.0626f;
xoff = (short)((320.0f - xfac * (3672.0f + 578.0f)) / 2.0f);
yoff = (short)((240.0f - yfac * (3612.0f + 418.0f)) / 2.0f);

// Now touch reads will work:
if (xpt2046_read()) {
    printf("Touch: %d, %d\n", TouchX, TouchY);  // Correct values
}
```

### Issue: Touch Coordinates Inverted or Offset

**Symptom**: Touch works but coordinates are flipped, mirrored, or offset.

**Cause**: Calibration values are for wrong orientation, or touch panel has different mapping.

**Solution 1**: Try inverted landscape mode:
```c
LCD_Set_Orientation(LCD_DISPLAY_ORIENTATION_LANDSCAPE_INVERTED);
```

**Solution 2**: Manually adjust calibration factors:
```c
// If X is inverted, negate xfac:
xfac = -0.0905f;

// If Y is inverted, negate yfac:
yfac = -0.0626f;

// If coordinates are offset, adjust xoff/yoff:
xoff += 20;  // Shift right
yoff -= 10;  // Shift up
```

**Solution 3**: Run full calibration:
```c
TP_Adjust();  // Interactive 4-point calibration
```

### Issue: No Touch Detection

**Symptom**: `xpt2046_read()` always returns `false`.

**Cause 1**: IRQ pin (GPIO36) is not going LOW when touched.

**Check**: Verify hardware connection on GPIO36.

**Cause 2**: `xpt2046_init()` not called.

**Solution**:
```c
xpt2046_init();  // Must call before reading!
```

**Cause 3**: Touch controller not responding on SPI.

**Check**: Verify GPIO25, GPIO32, GPIO33, GPIO39 connections.

### Issue: Build Errors - Component Not Found

**Symptom**:
```
CMake Error: Could not find package configuration file provided by "LCD"
```

**Cause**: CMakeLists.txt not configured correctly.

**Solution**: Verify both CMakeLists.txt files:

**Root `CMakeLists.txt`**:
```cmake
set(EXTRA_COMPONENT_DIRS "boards/marauder/components")
```

**Main `CMakeLists.txt`**:
```cmake
idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS ""
                    REQUIRES driver LCD GUI XPT2046)
```

### Issue: Undefined Reference Errors

**Symptom**:
```
undefined reference to `Init_LCD'
undefined reference to `xpt2046_init'
```

**Cause**: Missing includes or component requirements.

**Solution**:
```c
// Add includes:
#include "lcd.h"
#include "gui.h"
#include "xpt2046.h"

// Verify main/CMakeLists.txt:
REQUIRES driver LCD GUI XPT2046
```

### Issue: White Screen, No Display

**Symptom**: Backlight is on (white), but no text/graphics appear.

**Cause 1**: `Init_LCD()` not called.

**Solution**:
```c
Init_LCD(WHITE);  // Must call first!
```

**Cause 2**: Orientation changed after drawing.

**Solution**: Set orientation BEFORE drawing:
```c
Init_LCD(WHITE);
LCD_Set_Orientation(LCD_DISPLAY_ORIENTATION_LANDSCAPE);  // THEN draw
LCD_ShowString(100, 110, WHITE, BLACK, 24, "Hello", 0);
```

**Cause 3**: Text color matches background color.

**Solution**: Use contrasting colors:
```c
// BAD: White text on white background (invisible)
LCD_ShowString(10, 10, WHITE, WHITE, 24, "Text", 0);

// GOOD: Black text on white background
LCD_ShowString(10, 10, WHITE, BLACK, 24, "Text", 0);
```

---

## Quick Reference Checklist

### Minimal LCD Setup
```c
Init_LCD(WHITE);                                    // 1. Initialize
LCD_Set_Orientation(LCD_DISPLAY_ORIENTATION_LANDSCAPE);  // 2. Set orientation
LCD_ShowString(100, 110, WHITE, BLACK, 24, "Hello", 0);  // 3. Draw content
```

### Minimal Touch Setup
```c
xpt2046_init();                                     // 1. Initialize
xfac = 0.0905f; yfac = 0.0626f;                    // 2. Set calibration
xoff = -32; yoff = -6;                             // 3. Set offsets

while (1) {                                         // 4. Poll loop
    if (xpt2046_read()) {
        printf("Touch: %d, %d\n", TouchX, TouchY);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}
```

### Display + Touch Full Template
```c
void app_main(void) {
    // LCD
    Init_LCD(WHITE);
    LCD_Set_Orientation(LCD_DISPLAY_ORIENTATION_LANDSCAPE);
    LCD_ShowString(80, 10, WHITE, BLACK, 24, "Touch Demo", 0);

    // Touch
    xpt2046_init();
    xfac = 0.0905f; yfac = 0.0626f;
    xoff = (short)((320.0f - xfac * (3672.0f + 578.0f)) / 2.0f);
    yoff = (short)((240.0f - yfac * (3612.0f + 418.0f)) / 2.0f);

    // Main loop
    while (1) {
        if (xpt2046_read()) {
            if (TouchY < 120) {
                LCD_DrawFillRectangle(0, 0, 319, 239, BLUE);
            } else {
                LCD_DrawFillRectangle(0, 0, 319, 239, RED);
            }
            vTaskDelay(pdMS_TO_TICKS(2000));
            LCD_DrawFillRectangle(0, 0, 319, 239, WHITE);
            LCD_ShowString(80, 10, WHITE, BLACK, 24, "Touch Demo", 0);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

---

## API Function Summary

### LCD Functions (lcd.h)
| Function | Purpose | Performance |
|----------|---------|-------------|
| `Init_LCD(color)` | Initialize display | Slow (~2-3s) |
| `LCD_Set_Orientation(orient)` | Change screen rotation | Fast |
| `LCD_Clear(color)` | Fill screen (AVOID) | Very slow (~2-3s) |
| `LCD_WriteCMD(cmd)` | Send ILI9341 command | Fast |
| `LCD_WriteDate(data, len)` | Send raw data | Fast |

### Graphics Functions (gui.h)
| Function | Purpose | Performance |
|----------|---------|-------------|
| `LCD_DrawFillRectangle(x1,y1,x2,y2,color)` | Fill rectangle | Moderate |
| `LCD_ShowString(x,y,bg,fg,size,str,mode)` | Display text | Moderate |
| `LCD_DrawPoint(x,y,color)` | Draw pixel | Fast |
| `LCD_DrawLine(x1,y1,x2,y2,color)` | Draw line | Fast |
| `LCD_Draw_Circle(x,y,r,color)` | Circle outline | Moderate |
| `LCD_Draw_FillCircle(x,y,r,color)` | Filled circle | Moderate |

### Touch Functions (xpt2046.h)
| Function | Purpose | Notes |
|----------|---------|-------|
| `xpt2046_init()` | Initialize touch hardware | Call once |
| `xpt2046_read()` | Read touch state + coordinates | Returns bool |
| `TP_Adjust()` | Full calibration routine | Interactive, ~60s |
| `TP_Read_XOY(cmd)` | Read raw ADC value | Low-level |

---

## Hardware Reference

### Full Pin Summary Table

| Component | Pin | ESP32 GPIO | Function | Notes |
|-----------|-----|------------|----------|-------|
| **TFT LCD** | MOSI | GPIO12 | SPI data out | To display |
| | CLK | GPIO14 | SPI clock | 30 MHz |
| | CS | GPIO15 | Chip select | Active low |
| | DC | GPIO2 | Data/Command | 0=cmd, 1=data |
| | Backlight | GPIO21 | Backlight enable | Set HIGH |
| **Touch** | MOSI | GPIO32 | SPI data out | Separate SPI bus |
| | MISO | GPIO39 | SPI data in | Also IRQ input |
| | CLK | GPIO25 | SPI clock | 2 MHz |
| | CS | GPIO33 | Chip select | Active low |
| | IRQ | GPIO36 | Touch interrupt | Active low |

### Power Specifications
- **TFT LCD**: 40-100mA @ 3.3V (backlight on)
- **Touch Controller**: <1mA @ 3.3V
- **Total**: <110mA (well within USB power limits)

### Physical Dimensions
- **Screen Size**: 2.4" diagonal
- **Resolution**: 320x240 pixels
- **Pixel Pitch**: ~0.13mm
- **Active Area**: ~48.6mm x 36.5mm

---

## Revision History

| Date | Version | Changes |
|------|---------|---------|
| 2025-12-19 | 1.0 | Initial comprehensive guide |

---

**End of Document**

For additional reference, see:
- [LEVELLER_PINOUT.md](LEVELLER_PINOUT.md) - Full board pinout including MPU6050 and GPS
- [ESP32_CREEN.pdf](boards/marauder/docs/ESP32_CREEN.pdf) - Hardware schematic
- [ESP32_PINOUT_REFERENCE.md](boards/marauder/docs/ESP32_PINOUT_REFERENCE.md) - ESP32 GPIO reference
