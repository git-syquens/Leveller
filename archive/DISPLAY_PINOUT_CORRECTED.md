# Display Pinout Correction - ALI Purple Screen Board

## Issue Summary
The display was not working (black screen) despite software initializing correctly. Root cause: **incorrect pin definitions**.

## Incorrect Pinout (Old - WRONG)
Based on incorrect documentation, the following pins were used:
```c
#define LCD_PIN_MOSI        12
#define LCD_PIN_CLK         2
#define LCD_PIN_CS          13
#define LCD_PIN_DC          15
#define LCD_PIN_RST         14
// Missing: MISO and BCKL
```

## Correct Pinout (ALI Purple Screen Board - VERIFIED)
From working GitHub repository code:
```c
#define LCD_PIN_MISO        12  // ✅ VERIFIED
#define LCD_PIN_MOSI        13  // ✅ VERIFIED
#define LCD_PIN_CLK         14  // ✅ VERIFIED
#define LCD_PIN_CS          15  // ✅ VERIFIED
#define LCD_PIN_DC          2   // ✅ VERIFIED
#define LCD_PIN_RST         -1  // ✅ VERIFIED (not used)
#define LCD_PIN_BCKL        21  // ✅ VERIFIED (backlight control)
```

## Key Differences
| Signal | Old (Wrong) | New (Correct) | Change |
|--------|-------------|---------------|--------|
| MISO   | Not defined | GPIO12 | ➕ Added |
| MOSI   | GPIO12 | GPIO13 | ❌ Wrong pin |
| CLK    | GPIO2 | GPIO14 | ❌ Wrong pin |
| CS     | GPIO13 | GPIO15 | ❌ Wrong pin |
| DC     | GPIO15 | GPIO2 | ❌ Wrong pin |
| RST    | GPIO14 | -1 (disabled) | ❌ Wrong pin |
| BCKL   | Not defined | GPIO21 | ➕ Added (critical!) |

## Critical Fixes
1. **All pins were swapped** - DC and CLK were particularly confused
2. **Missing MISO** - GPIO12 needs to be defined even though it's input-only
3. **Missing backlight control** - GPIO21 must be driven HIGH for display to show anything

## Source of Correct Pinout
Verified from working code in:
- GitHub: https://github.com/git-syquens/Leveller
- File: `boards/marauder/components/LCD/include/lcd.h`
- Originally from ESP32-SCREEN (Marauder Board) schematics

## Board Variant Warning
⚠️ **IMPORTANT:** This pinout is specific to the **ALI Purple Screen Board** variant of the ESP32-SCREEN (Marauder Board). Other variants or documentation may show different pinouts, but for THIS specific hardware, the pinout above is correct and verified working.

## Status
✅ Display now working with moving "Hello World" demo
✅ FPS counter showing ~60 FPS
✅ Backlight control functioning
✅ Updated LEVELLER_PINOUT.md with verified pins
