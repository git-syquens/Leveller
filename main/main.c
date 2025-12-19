/**
 * @file main.c
 * @brief I2C Connection Test for Leveller Hardware
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "I2C_TEST";

// Pin definitions
#define I2C_SDA_PIN         GPIO_NUM_6
#define I2C_SCL_PIN         GPIO_NUM_7
#define I2C_FREQ_HZ         100000
#define I2C_PORT            I2C_NUM_0

// Expected I2C addresses
#define LCD_ADDR            0x3E
#define RGB_ADDR            0x62
#define MPU6050_ADDR        0x68

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

    ESP_LOGI(TAG, "I2C initialized - SDA: GPIO%d, SCL: GPIO%d", I2C_SDA_PIN, I2C_SCL_PIN);
    return ESP_OK;
}

/**
 * @brief Test I2C device at given address
 */
static bool i2c_test_device(uint8_t addr)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);

    return (ret == ESP_OK);
}

/**
 * @brief Scan entire I2C bus
 */
static void i2c_scan(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "================================");
    ESP_LOGI(TAG, "    I2C BUS SCAN");
    ESP_LOGI(TAG, "================================");
    ESP_LOGI(TAG, "Scanning 0x01-0x7F...");
    ESP_LOGI(TAG, "");

    int found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        if (i2c_test_device(addr)) {
            ESP_LOGI(TAG, "  [FOUND] Device at 0x%02X", addr);

            // Identify known devices
            if (addr == LCD_ADDR) {
                ESP_LOGI(TAG, "          --> JHD1313M3 LCD Controller");
            } else if (addr == RGB_ADDR) {
                ESP_LOGI(TAG, "          --> RGB Backlight Controller");
            } else if (addr == MPU6050_ADDR) {
                ESP_LOGI(TAG, "          --> MPU6050 Gyro/Accel");
            }
            found++;
        }
    }

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Scan complete: %d device(s) found", found);
    ESP_LOGI(TAG, "================================");
    ESP_LOGI(TAG, "");
}

/**
 * @brief Test LCD at 0x3E
 */
static void test_lcd(void)
{
    ESP_LOGI(TAG, "Testing LCD at 0x3E...");

    if (!i2c_test_device(LCD_ADDR)) {
        ESP_LOGE(TAG, "  LCD NOT FOUND!");
        return;
    }

    ESP_LOGI(TAG, "  LCD responds OK");

    // Try to send a simple command (clear display)
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LCD_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x80, true); // Command register
    i2c_master_write_byte(cmd, 0x01, true); // Clear display
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "  LCD command sent successfully");
    } else {
        ESP_LOGE(TAG, "  LCD command failed: %s", esp_err_to_name(ret));
    }
}

/**
 * @brief Test RGB backlight at 0x62
 */
static void test_rgb_backlight(void)
{
    ESP_LOGI(TAG, "Testing RGB backlight at 0x62...");

    if (!i2c_test_device(RGB_ADDR)) {
        ESP_LOGE(TAG, "  RGB controller NOT FOUND!");
        return;
    }

    ESP_LOGI(TAG, "  RGB controller responds OK");

    // Initialize PCA9633 RGB controller
    i2c_cmd_handle_t cmd;

    // MODE1: normal mode
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true); // MODE1 register
    i2c_master_write_byte(cmd, 0x00, true); // Normal mode
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    // MODE2
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x01, true); // MODE2 register
    i2c_master_write_byte(cmd, 0x01, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    // LEDOUT: enable all LEDs
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x08, true); // LEDOUT register
    i2c_master_write_byte(cmd, 0xAA, true); // All LEDs on
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    ESP_LOGI(TAG, "  Testing RED...");
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x02, true); // Red PWM
    i2c_master_write_byte(cmd, 255, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x03, true); // Green PWM
    i2c_master_write_byte(cmd, 0, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x04, true); // Blue PWM
    i2c_master_write_byte(cmd, 0, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    ESP_LOGI(TAG, "  RED backlight should be ON now!");
}

/**
 * @brief Test GPIO pins
 */
static void test_gpio(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "================================");
    ESP_LOGI(TAG, "    GPIO PIN TEST");
    ESP_LOGI(TAG, "================================");

    // Test SDA/SCL by reading their levels
    gpio_set_direction(I2C_SDA_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(I2C_SCL_PIN, GPIO_MODE_INPUT);

    int sda_level = gpio_get_level(I2C_SDA_PIN);
    int scl_level = gpio_get_level(I2C_SCL_PIN);

    ESP_LOGI(TAG, "GPIO6 (SDA) level: %d %s", sda_level, sda_level ? "HIGH" : "LOW");
    ESP_LOGI(TAG, "GPIO7 (SCL) level: %d %s", scl_level, scl_level ? "HIGH" : "LOW");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "NOTE: Both should be HIGH when idle (pullups)");
    ESP_LOGI(TAG, "      If LOW, check for short circuits");
    ESP_LOGI(TAG, "================================");
    ESP_LOGI(TAG, "");
}

void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  LEVELLER HARDWARE CONNECTION TEST");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Expected devices:");
    ESP_LOGI(TAG, "  - LCD at 0x3E (JHD1313M3)");
    ESP_LOGI(TAG, "  - RGB at 0x62 (PCA9633)");
    ESP_LOGI(TAG, "  - MPU6050 at 0x68 (optional)");
    ESP_LOGI(TAG, "");

    vTaskDelay(pdMS_TO_TICKS(1000));

    // Test GPIO first
    test_gpio();

    // Initialize I2C
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "FATAL: I2C initialization failed!");
        ESP_LOGE(TAG, "Check wiring:");
        ESP_LOGE(TAG, "  - GPIO6 = SDA");
        ESP_LOGE(TAG, "  - GPIO7 = SCL");
        ESP_LOGE(TAG, "  - 3V3 connected to all VCC");
        ESP_LOGE(TAG, "  - GND connected to all GND");
        while(1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    vTaskDelay(pdMS_TO_TICKS(500));

    // Scan I2C bus
    i2c_scan();

    vTaskDelay(pdMS_TO_TICKS(1000));

    // Test LCD
    ESP_LOGI(TAG, "");
    test_lcd();

    vTaskDelay(pdMS_TO_TICKS(1000));

    // Test RGB backlight
    ESP_LOGI(TAG, "");
    test_rgb_backlight();

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  TEST COMPLETE");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "RESULTS:");
    ESP_LOGI(TAG, "  - Check serial output for I2C scan results");
    ESP_LOGI(TAG, "  - LCD backlight should be RED if working");
    ESP_LOGI(TAG, "  - If no devices found, check connections!");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Looping color test...");

    // Continuous color cycling test
    while (1) {
        if (i2c_test_device(RGB_ADDR)) {
            // Red
            ESP_LOGI(TAG, "Setting RED...");
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
            i2c_master_write_byte(cmd, 0x02, true);
            i2c_master_write_byte(cmd, 255, true);
            i2c_master_stop(cmd);
            i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
            i2c_cmd_link_delete(cmd);

            cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
            i2c_master_write_byte(cmd, 0x03, true);
            i2c_master_write_byte(cmd, 0, true);
            i2c_master_stop(cmd);
            i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
            i2c_cmd_link_delete(cmd);

            cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
            i2c_master_write_byte(cmd, 0x04, true);
            i2c_master_write_byte(cmd, 0, true);
            i2c_master_stop(cmd);
            i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
            i2c_cmd_link_delete(cmd);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));

        if (i2c_test_device(RGB_ADDR)) {
            // Green
            ESP_LOGI(TAG, "Setting GREEN...");
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
            i2c_master_write_byte(cmd, 0x02, true);
            i2c_master_write_byte(cmd, 0, true);
            i2c_master_stop(cmd);
            i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
            i2c_cmd_link_delete(cmd);

            cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
            i2c_master_write_byte(cmd, 0x03, true);
            i2c_master_write_byte(cmd, 255, true);
            i2c_master_stop(cmd);
            i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
            i2c_cmd_link_delete(cmd);

            cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
            i2c_master_write_byte(cmd, 0x04, true);
            i2c_master_write_byte(cmd, 0, true);
            i2c_master_stop(cmd);
            i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
            i2c_cmd_link_delete(cmd);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));

        if (i2c_test_device(RGB_ADDR)) {
            // Blue
            ESP_LOGI(TAG, "Setting BLUE...");
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
            i2c_master_write_byte(cmd, 0x02, true);
            i2c_master_write_byte(cmd, 0, true);
            i2c_master_stop(cmd);
            i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
            i2c_cmd_link_delete(cmd);

            cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
            i2c_master_write_byte(cmd, 0x03, true);
            i2c_master_write_byte(cmd, 0, true);
            i2c_master_stop(cmd);
            i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
            i2c_cmd_link_delete(cmd);

            cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
            i2c_master_write_byte(cmd, 0x04, true);
            i2c_master_write_byte(cmd, 255, true);
            i2c_master_stop(cmd);
            i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
            i2c_cmd_link_delete(cmd);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
