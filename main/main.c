/**
 * @file main.c
 * @brief Leveller - Camper Levelling Indicator Main Application
 */

#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "jhd1313m3.h"

static const char *TAG = "Leveller";

// Pin definitions (from LEVELLER_PINOUT.md)
#define I2C_SDA_PIN         GPIO_NUM_6
#define I2C_SCL_PIN         GPIO_NUM_7
#define I2C_FREQ_HZ         100000
#define I2C_PORT            I2C_NUM_0

#define MODE_BUTTON_PIN     GPIO_NUM_4
#define FUNC_BUTTON_PIN     GPIO_NUM_5
#define STATUS_LED_PIN      GPIO_NUM_8

#define MPU6050_ADDR        0x68

// Global handles
static jhd1313m3_handle_t *lcd_handle = NULL;

/**
 * @brief Initialize I2C bus
 */
static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ,
    };

    esp_err_t err = i2c_param_config(I2C_PORT, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C param config failed: %s", esp_err_to_name(err));
        return err;
    }

    err = i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "I2C initialized on SDA=%d, SCL=%d", I2C_SDA_PIN, I2C_SCL_PIN);
    return ESP_OK;
}

/**
 * @brief Scan I2C bus for devices
 */
static void i2c_scan(void)
{
    ESP_LOGI(TAG, "Scanning I2C bus...");

    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "  Found device at 0x%02X", addr);
        }
    }

    ESP_LOGI(TAG, "I2C scan complete");
}

/**
 * @brief Initialize GPIO buttons
 */
static void gpio_init_buttons(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << MODE_BUTTON_PIN) | (1ULL << FUNC_BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&io_conf);
    ESP_LOGI(TAG, "Buttons initialized on GPIO%d and GPIO%d", MODE_BUTTON_PIN, FUNC_BUTTON_PIN);
}

/**
 * @brief Initialize LCD display
 */
static esp_err_t lcd_init(void)
{
    jhd1313m3_config_t lcd_config = {
        .i2c_port = I2C_PORT,
        .lcd_addr = JHD1313M3_LCD_ADDR,
        .rgb_addr = JHD1313M3_RGB_ADDR,
    };

    esp_err_t ret = jhd1313m3_init(&lcd_config, &lcd_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LCD init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Display startup message
    jhd1313m3_clear(lcd_handle);
    jhd1313m3_set_cursor(lcd_handle, 0, 0);
    jhd1313m3_write_string(lcd_handle, "  LEVELLER");
    jhd1313m3_set_cursor(lcd_handle, 0, 1);
    jhd1313m3_write_string(lcd_handle, " Initializing...");
    jhd1313m3_set_rgb(lcd_handle, 0, 128, 255); // Blue

    ESP_LOGI(TAG, "LCD initialized");
    return ESP_OK;
}

/**
 * @brief Main application
 */
void app_main(void)
{
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "Leveller - Camper Level Indicator");
    ESP_LOGI(TAG, "=================================");

    // Initialize NVS (for storing calibration data)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize I2C
    ESP_ERROR_CHECK(i2c_master_init());

    // Scan for I2C devices
    i2c_scan();

    // Initialize buttons
    gpio_init_buttons();

    // Initialize LCD
    ESP_ERROR_CHECK(lcd_init());

    vTaskDelay(pdMS_TO_TICKS(2000));

    // Update display
    jhd1313m3_clear(lcd_handle);
    jhd1313m3_set_cursor(lcd_handle, 0, 0);
    jhd1313m3_write_string(lcd_handle, "Pitch:  0.0");
    jhd1313m3_write_char(lcd_handle, 0xDF); // Degree symbol
    jhd1313m3_set_cursor(lcd_handle, 0, 1);
    jhd1313m3_write_string(lcd_handle, "Roll:   0.0");
    jhd1313m3_write_char(lcd_handle, 0xDF); // Degree symbol
    jhd1313m3_set_rgb(lcd_handle, 0, 255, 0); // Green = level

    ESP_LOGI(TAG, "System ready!");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Next steps:");
    ESP_LOGI(TAG, "  1. Add MPU6050 driver integration");
    ESP_LOGI(TAG, "  2. Implement angle calculation");
    ESP_LOGI(TAG, "  3. Add button handling for mode switching");
    ESP_LOGI(TAG, "  4. Implement calibration routine");

    // Main loop
    uint8_t counter = 0;
    while (1) {
        // Read button states
        int mode_btn = gpio_get_level(MODE_BUTTON_PIN);
        int func_btn = gpio_get_level(FUNC_BUTTON_PIN);

        if (mode_btn == 0) { // Button pressed (active low)
            ESP_LOGI(TAG, "Mode button pressed");
            jhd1313m3_set_rgb(lcd_handle, 255, 255, 0); // Yellow
        }
        if (func_btn == 0) { // Button pressed (active low)
            ESP_LOGI(TAG, "Function button pressed");
            jhd1313m3_set_rgb(lcd_handle, 255, 0, 255); // Magenta
        }

        // Simulate angle update (demo)
        if (counter % 5 == 0) {
            char line[17];
            float demo_pitch = (counter % 10) - 5;
            float demo_roll = ((counter * 2) % 10) - 5;

            jhd1313m3_set_cursor(lcd_handle, 0, 0);
            snprintf(line, sizeof(line), "Pitch: %+5.1f", demo_pitch);
            jhd1313m3_write_string(lcd_handle, line);
            jhd1313m3_write_char(lcd_handle, 0xDF);

            jhd1313m3_set_cursor(lcd_handle, 0, 1);
            snprintf(line, sizeof(line), "Roll:  %+5.1f", demo_roll);
            jhd1313m3_write_string(lcd_handle, line);
            jhd1313m3_write_char(lcd_handle, 0xDF);

            // Color based on max angle
            float max_angle = fabs(demo_pitch) > fabs(demo_roll) ? fabs(demo_pitch) : fabs(demo_roll);
            if (max_angle < 0.5) {
                jhd1313m3_set_rgb(lcd_handle, 0, 255, 0); // Green
            } else if (max_angle < 2.0) {
                jhd1313m3_set_rgb(lcd_handle, 255, 255, 0); // Yellow
            } else {
                jhd1313m3_set_rgb(lcd_handle, 255, 0, 0); // Red
            }
        }

        counter++;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
