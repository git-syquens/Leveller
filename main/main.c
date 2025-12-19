/*
 * Leveller - High-Performance LVGL Demo
 *
 * Features:
 * - White background with moving "Hello World" text sprite
 * - FPS counter displayed on serial output
 * - Touch interaction: Top half = blue, Bottom half = red (1 second)
 * - Touch coordinates logged to serial
 * - Landscape orientation (320x240)
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"
#include "esp_lcd_touch_xpt2046.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"

static const char *TAG = "LEVELLER";

/* Pin Definitions (from working GitHub code) */
// TFT LCD Display (SPI2)
#define LCD_PIN_MISO        12
#define LCD_PIN_MOSI        13
#define LCD_PIN_CLK         14
#define LCD_PIN_CS          15
#define LCD_PIN_DC          2
#define LCD_PIN_RST         -1
#define LCD_PIN_BCKL        21  // Backlight control

// Touch Controller (XPT2046 - SPI3)
#define TOUCH_PIN_MOSI      32
#define TOUCH_PIN_MISO      36
#define TOUCH_PIN_CLK       25
#define TOUCH_PIN_CS        33
#define TOUCH_PIN_IRQ       39

// Display Configuration (Landscape)
#define LCD_H_RES           320
#define LCD_V_RES           240
#define LCD_PIXEL_CLOCK_HZ  (40 * 1000 * 1000)  // 40MHz
#define LCD_CMD_BITS        8
#define LCD_PARAM_BITS      8

// Touch Configuration
#define TOUCH_SPI_CLOCK_HZ  (2 * 1000 * 1000)   // 2MHz

// LVGL Configuration
#define LVGL_BUFFER_HEIGHT  40  // 40 lines buffer

/* Global Handles */
static esp_lcd_panel_handle_t lcd_panel = NULL;
static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;
static lv_obj_t *moving_label = NULL;
static lv_obj_t *screen = NULL;

/* Animation State */
static int16_t text_x = 0;
static int16_t text_y = 120;
static int16_t text_dx = 2;  // X velocity
static int16_t text_dy = 1;  // Y velocity

/* FPS Tracking */
static uint32_t frame_count = 0;
static int64_t last_fps_time = 0;
static float current_fps = 0.0f;

/* Touch State */
typedef enum {
    SCREEN_STATE_WHITE,
    SCREEN_STATE_BLUE,
    SCREEN_STATE_RED
} screen_state_t;

static screen_state_t screen_state = SCREEN_STATE_WHITE;
static int64_t state_change_time = 0;

/**
 * @brief Initialize NVS Flash
 */
static esp_err_t init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

/**
 * @brief Initialize LCD Display (ILI9341)
 */
static esp_err_t init_lcd(void)
{
    ESP_LOGI(TAG, "Initializing LCD display (ILI9341)...");

    // SPI bus configuration for LCD
    spi_bus_config_t buscfg = {
        .mosi_io_num = LCD_PIN_MOSI,
        .miso_io_num = LCD_PIN_MISO,
        .sclk_io_num = LCD_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * LVGL_BUFFER_HEIGHT * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Enable backlight
    gpio_set_direction(LCD_PIN_BCKL, GPIO_MODE_OUTPUT);
    gpio_set_level(LCD_PIN_BCKL, 1);  // Turn on backlight

    // LCD panel IO configuration
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_PIN_DC,
        .cs_gpio_num = LCD_PIN_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &lcd_io));

    // LCD panel configuration
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(lcd_io, &panel_config, &lcd_panel));

    // Initialize LCD panel
    ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(lcd_panel, true));

    // Set to landscape mode (320x240)
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(lcd_panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(lcd_panel, false, true));

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_panel, true));

    ESP_LOGI(TAG, "LCD initialized successfully (320x240 landscape)");
    return ESP_OK;
}

/**
 * @brief Initialize Touch Controller (XPT2046)
 */
static esp_err_t init_touch(void)
{
    ESP_LOGI(TAG, "Initializing touch controller (XPT2046)...");

    // SPI bus configuration for touch (separate bus)
    spi_bus_config_t buscfg = {
        .mosi_io_num = TOUCH_PIN_MOSI,
        .miso_io_num = TOUCH_PIN_MISO,
        .sclk_io_num = TOUCH_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Touch panel IO configuration
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_spi_config_t tp_io_config = ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG(TOUCH_PIN_CS);
    tp_io_config.spi_mode = 0;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI3_HOST, &tp_io_config, &tp_io_handle));

    // Touch panel configuration
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_H_RES,
        .y_max = LCD_V_RES,
        .rst_gpio_num = -1,
        .int_gpio_num = TOUCH_PIN_IRQ,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 1,
            .mirror_x = 0,
            .mirror_y = 1,
        },
    };
    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &tp_cfg, &touch_handle));

    ESP_LOGI(TAG, "Touch controller initialized successfully");
    return ESP_OK;
}

/**
 * @brief Initialize LVGL with esp_lvgl_port
 */
static esp_err_t init_lvgl(void)
{
    ESP_LOGI(TAG, "Initializing LVGL...");

    // LVGL port configuration
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    // Add LCD screen
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io,
        .panel_handle = lcd_panel,
        .buffer_size = LCD_H_RES * LVGL_BUFFER_HEIGHT,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
        }
    };
    lv_disp_t *disp = lvgl_port_add_disp(&disp_cfg);

    // Add touch input device
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp,
        .handle = touch_handle,
    };
    lvgl_port_add_touch(&touch_cfg);

    ESP_LOGI(TAG, "LVGL initialized successfully");
    return ESP_OK;
}

/**
 * @brief Update FPS counter
 */
static void update_fps(void)
{
    frame_count++;

    int64_t current_time = esp_timer_get_time();
    int64_t elapsed = current_time - last_fps_time;

    // Update FPS every second
    if (elapsed >= 1000000) {  // 1 second in microseconds
        current_fps = (float)frame_count * 1000000.0f / (float)elapsed;
        ESP_LOGI(TAG, "FPS: %.2f", current_fps);

        frame_count = 0;
        last_fps_time = current_time;
    }
}

/**
 * @brief Touch event callback
 */
static void touch_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING) {
        lv_indev_t *indev = lv_event_get_indev(e);
        lv_point_t point;
        lv_indev_get_point(indev, &point);

        ESP_LOGI(TAG, "Touch at X=%d, Y=%d", point.x, point.y);

        // Determine touch region
        if (point.y < (LCD_V_RES / 2)) {
            // Top half - Blue
            if (screen_state != SCREEN_STATE_BLUE) {
                ESP_LOGI(TAG, "TOP half touched - setting BLUE");
                lv_obj_set_style_bg_color(screen, lv_color_hex(0x0000FF), 0);
                screen_state = SCREEN_STATE_BLUE;
                state_change_time = esp_timer_get_time();
            }
        } else {
            // Bottom half - Red
            if (screen_state != SCREEN_STATE_RED) {
                ESP_LOGI(TAG, "BOTTOM half touched - setting RED");
                lv_obj_set_style_bg_color(screen, lv_color_hex(0xFF0000), 0);
                screen_state = SCREEN_STATE_RED;
                state_change_time = esp_timer_get_time();
            }
        }
    }
}

/**
 * @brief Create UI with moving text sprite
 */
static void create_ui(void)
{
    if (lvgl_port_lock(0)) {
        screen = lv_scr_act();

        // Set white background
        lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFFFF), 0);

        // Add touch event handler to screen
        lv_obj_add_event_cb(screen, touch_event_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(screen, touch_event_cb, LV_EVENT_PRESSING, NULL);
        lv_obj_add_flag(screen, LV_OBJ_FLAG_CLICKABLE);

        // Create moving label (sprite)
        moving_label = lv_label_create(screen);
        lv_label_set_text(moving_label, "Hello World");
        lv_obj_set_style_text_color(moving_label, lv_color_hex(0x000000), 0);
        lv_obj_set_style_text_font(moving_label, &lv_font_montserrat_32, 0);

        // Initial position
        lv_obj_set_pos(moving_label, text_x, text_y);

        lvgl_port_unlock();
    }

    ESP_LOGI(TAG, "UI created - white background with moving text");
}

/**
 * @brief Animation task - updates moving text sprite
 */
static void animation_task(void *pvParameters)
{
    const TickType_t delay = pdMS_TO_TICKS(16);  // ~60 FPS target

    // Get label dimensions
    lv_coord_t label_width = 0;
    lv_coord_t label_height = 0;

    while (1) {
        if (lvgl_port_lock(0)) {
            // Get label dimensions (once)
            if (label_width == 0) {
                label_width = lv_obj_get_width(moving_label);
                label_height = lv_obj_get_height(moving_label);
            }

            // Update position
            text_x += text_dx;
            text_y += text_dy;

            // Bounce off edges
            if (text_x <= 0 || text_x >= (LCD_H_RES - label_width)) {
                text_dx = -text_dx;
                text_x += text_dx;  // Correct position
            }

            if (text_y <= 0 || text_y >= (LCD_V_RES - label_height)) {
                text_dy = -text_dy;
                text_y += text_dy;  // Correct position
            }

            // Apply new position
            lv_obj_set_pos(moving_label, text_x, text_y);

            // Update FPS
            update_fps();

            lvgl_port_unlock();
        }

        vTaskDelay(delay);
    }
}

/**
 * @brief State management task - handles color reversion
 */
static void state_task(void *pvParameters)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));  // Check every 100ms

        int64_t current_time = esp_timer_get_time();
        int64_t elapsed = current_time - state_change_time;

        // Revert to white after 1 second
        if (screen_state != SCREEN_STATE_WHITE && elapsed >= 1000000) {
            if (lvgl_port_lock(0)) {
                ESP_LOGI(TAG, "Reverting to WHITE background");
                lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFFFF), 0);
                screen_state = SCREEN_STATE_WHITE;
                lvgl_port_unlock();
            }
        }
    }
}

/**
 * @brief Application entry point
 */
void app_main(void)
{
    ESP_LOGI(TAG, "=== Leveller - High-Performance Demo ===");
    ESP_LOGI(TAG, "Features:");
    ESP_LOGI(TAG, "  - Moving 'Hello World' sprite");
    ESP_LOGI(TAG, "  - FPS counter on serial");
    ESP_LOGI(TAG, "  - Touch top half = BLUE (1 sec)");
    ESP_LOGI(TAG, "  - Touch bottom half = RED (1 sec)");
    ESP_LOGI(TAG, "  - Landscape 320x240");

    // Initialize NVS
    ESP_ERROR_CHECK(init_nvs());

    // Initialize LCD display
    ESP_ERROR_CHECK(init_lcd());

    // Initialize touch controller
    ESP_ERROR_CHECK(init_touch());

    // Initialize LVGL
    ESP_ERROR_CHECK(init_lvgl());

    // Create UI
    create_ui();

    // Initialize FPS timer
    last_fps_time = esp_timer_get_time();

    // Start animation task
    xTaskCreate(animation_task, "animation", 4096, NULL, 5, NULL);

    // Start state management task
    xTaskCreate(state_task, "state", 2048, NULL, 4, NULL);

    ESP_LOGI(TAG, "System ready - watch serial for FPS and touch coordinates");
}
