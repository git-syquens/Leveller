/**
 * @file demo.c
 * @brief Demo mode implementation for testing Leveller hardware modules
 */

#include "demo.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "DEMO";

// I2C addresses
#define LCD_ADDR 0x3E
#define RGB_ADDR 0x62

// I2C port (set via demo_init)
static i2c_port_t demo_i2c_port = I2C_NUM_0;

/**
 * @brief Test if I2C device responds at given address
 */
static bool i2c_test_device(uint8_t addr)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(demo_i2c_port, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);

    return (ret == ESP_OK);
}

/**
 * @brief Send command to LCD
 * Based on Grove LCD RGB Backlight examples
 * Control byte: 0x80 = Co=1 (more data follows), RS=0 (command mode)
 */
static esp_err_t lcd_send_command(uint8_t cmd)
{
    i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (LCD_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(i2c_cmd, 0x80, true); // Control byte: Co=1, RS=0
    i2c_master_write_byte(i2c_cmd, cmd, true);
    i2c_master_stop(i2c_cmd);
    esp_err_t ret = i2c_master_cmd_begin(demo_i2c_port, i2c_cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(i2c_cmd);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LCD command 0x%02X failed: %s", cmd, esp_err_to_name(ret));
    }
    return ret;
}

/**
 * @brief Send data (character) to LCD
 * Based on Grove LCD RGB Backlight examples
 * Control byte: 0x40 = Co=0 (last byte), RS=1 (data mode)
 */
static esp_err_t lcd_send_data(uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LCD_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x40, true); // Control byte: Co=0, RS=1
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(demo_i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LCD data 0x%02X ('%c') failed: %s", data,
                 (data >= 32 && data < 127) ? data : '?', esp_err_to_name(ret));
    }
    return ret;
}

/**
 * @brief Initialize LCD display (JHD1313M3 - ST7066U compatible)
 */
static void lcd_init(void)
{
    ESP_LOGI(TAG, "LCD Init: Starting initialization sequence...");

    // Wait for LCD power-on reset to complete (> 40ms after VCC reaches 4.5V)
    vTaskDelay(pdMS_TO_TICKS(50));

    // Step 1: Function set - 8-bit mode, 2 lines, 5x8 font (Normal instruction table)
    ESP_LOGI(TAG, "LCD Init: Step 1 - Function set 0x38");
    lcd_send_command(0x38);
    vTaskDelay(pdMS_TO_TICKS(1));

    // Step 2: Switch to extended instruction table
    ESP_LOGI(TAG, "LCD Init: Step 2 - Extended instruction table 0x39");
    lcd_send_command(0x39);
    vTaskDelay(pdMS_TO_TICKS(1));

    // Step 3: Internal OSC frequency (BS=1, F2-F0=100 -> 183Hz)
    ESP_LOGI(TAG, "LCD Init: Step 3 - OSC frequency 0x14");
    lcd_send_command(0x14);
    vTaskDelay(pdMS_TO_TICKS(1));

    // Step 4: Contrast set (C3-C0 bits) - middle value
    ESP_LOGI(TAG, "LCD Init: Step 4 - Contrast 0x78");
    lcd_send_command(0x78); // Increased from 0x70 for better visibility
    vTaskDelay(pdMS_TO_TICKS(1));

    // Step 5: Power/ICON/Contrast control (Ion=1, Bon=1, C5-C4=10)
    ESP_LOGI(TAG, "LCD Init: Step 5 - Power control 0x5E");
    lcd_send_command(0x5E); // Changed from 0x56 - boost on, contrast bits
    vTaskDelay(pdMS_TO_TICKS(1));

    // Step 6: Follower control (Fon=1, Rab2-Rab0=100)
    ESP_LOGI(TAG, "LCD Init: Step 6 - Follower control 0x6D");
    lcd_send_command(0x6D); // Changed from 0x6C for better follower ratio
    vTaskDelay(pdMS_TO_TICKS(200)); // Wait for power stabilization

    // Step 7: Switch back to normal instruction table
    ESP_LOGI(TAG, "LCD Init: Step 7 - Normal instruction table 0x38");
    lcd_send_command(0x38);
    vTaskDelay(pdMS_TO_TICKS(1));

    // Step 8: Display ON, cursor OFF, blink OFF
    ESP_LOGI(TAG, "LCD Init: Step 8 - Display ON 0x0C");
    lcd_send_command(0x0C);
    vTaskDelay(pdMS_TO_TICKS(1));

    // Step 9: Entry mode set - increment, no shift
    ESP_LOGI(TAG, "LCD Init: Step 9 - Entry mode 0x06");
    lcd_send_command(0x06);
    vTaskDelay(pdMS_TO_TICKS(1));

    // Step 10: Clear display
    ESP_LOGI(TAG, "LCD Init: Step 10 - Clear display 0x01");
    lcd_send_command(0x01);
    vTaskDelay(pdMS_TO_TICKS(2));

    ESP_LOGI(TAG, "LCD Init: Complete!");
}

/**
 * @brief Clear LCD display
 */
static void lcd_clear(void)
{
    lcd_send_command(0x01);
    vTaskDelay(pdMS_TO_TICKS(2));
}

/**
 * @brief Set cursor position on LCD
 * @param row Row number (0-1)
 * @param col Column number (0-15)
 */
static void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t addr = (row == 0) ? 0x80 : 0xC0;
    lcd_send_command(addr + col);
}

/**
 * @brief Print string to LCD
 */
static void lcd_print(const char* str)
{
    while (*str) {
        lcd_send_data(*str++);
    }
}

/**
 * @brief Set RGB backlight color
 * @param red Red value (0-255)
 * @param green Green value (0-255)
 * @param blue Blue value (0-255)
 */
static void set_rgb_color(uint8_t red, uint8_t green, uint8_t blue)
{
    if (!i2c_test_device(RGB_ADDR)) {
        ESP_LOGW(TAG, "RGB controller not responding");
        return;
    }

    i2c_cmd_handle_t cmd;

    // Set red
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x02, true); // Red PWM register
    i2c_master_write_byte(cmd, red, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(demo_i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    // Set green
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x03, true); // Green PWM register
    i2c_master_write_byte(cmd, green, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(demo_i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    // Set blue
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (RGB_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x04, true); // Blue PWM register
    i2c_master_write_byte(cmd, blue, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(demo_i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
}

/**
 * @brief Display banner with color name on LCD
 */
static void display_color_banner(const char* color_name, const char* line2)
{
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("COLOR TEST:");
    lcd_set_cursor(1, 0);
    lcd_print(color_name);
    if (line2) {
        size_t len = strlen(color_name);
        if (len < 16) {
            lcd_set_cursor(1, len + 1);
            lcd_print(line2);
        }
    }
}

void demo_init(i2c_port_t i2c_port)
{
    demo_i2c_port = i2c_port;
    ESP_LOGI(TAG, "Demo module initialized (I2C port %d)", i2c_port);

    // Set green backlight first (we know this works)
    ESP_LOGI(TAG, "Setting GREEN backlight...");
    set_rgb_color(0, 255, 0);
    vTaskDelay(pdMS_TO_TICKS(500));

    // Initialize LCD if available
    if (i2c_test_device(LCD_ADDR)) {
        ESP_LOGI(TAG, "LCD detected at 0x%02X", LCD_ADDR);
        ESP_LOGI(TAG, "Initializing LCD...");
        lcd_init();

        // Simple test - just write "HELLO" without clearing first
        ESP_LOGI(TAG, "Writing HELLO to LCD...");
        lcd_send_data('H');
        lcd_send_data('E');
        lcd_send_data('L');
        lcd_send_data('L');
        lcd_send_data('O');

        ESP_LOGI(TAG, "Done - check LCD for HELLO text");
        vTaskDelay(pdMS_TO_TICKS(5000)); // Wait 5 seconds so you can see it

        ESP_LOGI(TAG, "LCD test complete");
    } else {
        ESP_LOGW(TAG, "LCD not found at 0x%02X - text display disabled", LCD_ADDR);
    }
}

void demo_run_rgb_color_rotation(void)
{
    ESP_LOGI(TAG, "Starting RGB color rotation demo...");
    ESP_LOGI(TAG, "Press RESET to exit demo mode");

    bool lcd_available = i2c_test_device(LCD_ADDR);

    while (1) {
        // Red
        ESP_LOGI(TAG, "Setting RED...");
        if (lcd_available) display_color_banner("RED", NULL);
        set_rgb_color(255, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Green
        ESP_LOGI(TAG, "Setting GREEN...");
        if (lcd_available) display_color_banner("GREEN", NULL);
        set_rgb_color(0, 255, 0);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Blue
        ESP_LOGI(TAG, "Setting BLUE...");
        if (lcd_available) display_color_banner("BLUE", NULL);
        set_rgb_color(0, 0, 255);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Yellow
        ESP_LOGI(TAG, "Setting YELLOW...");
        if (lcd_available) display_color_banner("YELLOW", NULL);
        set_rgb_color(255, 255, 0);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Cyan
        ESP_LOGI(TAG, "Setting CYAN...");
        if (lcd_available) display_color_banner("CYAN", NULL);
        set_rgb_color(0, 255, 255);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Magenta
        ESP_LOGI(TAG, "Setting MAGENTA...");
        if (lcd_available) display_color_banner("MAGENTA", NULL);
        set_rgb_color(255, 0, 255);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // White
        ESP_LOGI(TAG, "Setting WHITE...");
        if (lcd_available) display_color_banner("WHITE", NULL);
        set_rgb_color(255, 255, 255);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Off (black)
        ESP_LOGI(TAG, "Setting OFF...");
        if (lcd_available) display_color_banner("OFF", "(BLACK)");
        set_rgb_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
