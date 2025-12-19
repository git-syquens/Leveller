/**
 * @file main.c
 * @brief TFT LCD Test - Hello World with Touch on ESP32-SCREEN
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lcd.h"
#include "gui.h"
#include "xpt2046.h"

static const char *TAG = "MAIN";

// External touch coordinates from xpt2046 driver
extern uint16_t TouchX;
extern uint16_t TouchY;

// External calibration factors from xpt2046 driver
extern float xfac;
extern float yfac;
extern short xoff;
extern short yoff;

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-SCREEN TFT LCD + Touch Test");
    ESP_LOGI(TAG, "Initializing TFT display...");

    // Initialize LCD - now defaults to LANDSCAPE mode and clears to WHITE
    // (Modified lcd.c to initialize in landscape instead of portrait)
    Init_LCD(WHITE);

    ESP_LOGI(TAG, "TFT initialized in LANDSCAPE (320x240)");

    // Force complete display refresh by toggling display off/on
    LCD_WriteCMD(0x28);  // Display OFF
    vTaskDelay(pdMS_TO_TICKS(100));
    LCD_WriteCMD(0x29);  // Display ON

    ESP_LOGI(TAG, "Display refresh complete");

    // Display "Hello World" in the center
    LCD_ShowString(100, 110, WHITE, BLACK, 24, "Hello World", 0);

    ESP_LOGI(TAG, "Text displayed: 'Hello World'");

    // Initialize touch controller
    xpt2046_init();
    ESP_LOGI(TAG, "Touch controller initialized");

    // Set calibration values manually (from header file reference values)
    // Based on landscape orientation (320x240)
    // Reference calibration points from xpt2046.h comments:
    // Point1(top-left): ADX=3672, ADY=513
    // Point3(bottom-left): ADX=578, ADY=418
    // Point4(bottom-right): ADX=540, ADY=3612
    //
    // Formula: xfac = (320-40) / (3672-578) = 280/3094 = 0.0905
    //          yfac = (240-40) / (3612-418) = 200/3194 = 0.0626
    //          xoff = (320 - xfac*(3672+578))/2
    //          yoff = (240 - yfac*(3612+418))/2

    xfac = 0.0905f;
    yfac = 0.0626f;
    xoff = (short)((320.0f - xfac * (3672.0f + 578.0f)) / 2.0f);
    yoff = (short)((240.0f - yfac * (3612.0f + 418.0f)) / 2.0f);

    ESP_LOGI(TAG, "Touch calibration set: xfac=%.4f, yfac=%.4f, xoff=%d, yoff=%d",
             xfac, yfac, xoff, yoff);

    ESP_LOGI(TAG, "Touch test active:");
    ESP_LOGI(TAG, "  - Touch TOP half -> Blue for 2 seconds");
    ESP_LOGI(TAG, "  - Touch BOTTOM half -> Red for 2 seconds");

    // Main loop - check for touch events
    while (1) {
        // Check if screen is touched
        if (xpt2046_read()) {
            ESP_LOGI(TAG, "Touch detected at X=%d, Y=%d", TouchX, TouchY);

            // Landscape mode: 320x240, so Y=120 is the middle
            if (TouchY < 120) {
                // Top half touched - turn screen BLUE
                ESP_LOGI(TAG, "TOP touched - turning BLUE");
                LCD_DrawFillRectangle(0, 0, 319, 239, BLUE);
                vTaskDelay(pdMS_TO_TICKS(2000));

                // Return to white with text
                LCD_DrawFillRectangle(0, 0, 319, 239, WHITE);
                LCD_ShowString(100, 110, WHITE, BLACK, 24, "Hello World", 0);
                ESP_LOGI(TAG, "Back to WHITE");
            } else {
                // Bottom half touched - turn screen RED
                ESP_LOGI(TAG, "BOTTOM touched - turning RED");
                LCD_DrawFillRectangle(0, 0, 319, 239, RED);
                vTaskDelay(pdMS_TO_TICKS(2000));

                // Return to white with text
                LCD_DrawFillRectangle(0, 0, 319, 239, WHITE);
                LCD_ShowString(100, 110, WHITE, BLACK, 24, "Hello World", 0);
                ESP_LOGI(TAG, "Back to WHITE");
            }
        }

        // Small delay to avoid overwhelming the touch controller
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
