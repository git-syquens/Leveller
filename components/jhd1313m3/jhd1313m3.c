/**
 * @file jhd1313m3.c
 * @brief JHD1313M3 RGB LCD Display Driver Implementation
 */

#include "jhd1313m3.h"
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "JHD1313M3";

// I2C timeout
#define I2C_TIMEOUT_MS          1000

// LCD command/data register select
#define LCD_REG_CMD             0x80
#define LCD_REG_DATA            0x40

/**
 * @brief Write byte to I2C device
 */
static esp_err_t i2c_write_byte(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    return ret;
}

/**
 * @brief Send command to LCD
 */
static esp_err_t lcd_send_command(jhd1313m3_handle_t *handle, uint8_t cmd)
{
    esp_err_t ret = i2c_write_byte(handle->i2c_port, handle->lcd_addr, LCD_REG_CMD, cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send command 0x%02x", cmd);
    }
    vTaskDelay(pdMS_TO_TICKS(2)); // Command execution delay
    return ret;
}

/**
 * @brief Send data to LCD
 */
static esp_err_t lcd_send_data(jhd1313m3_handle_t *handle, uint8_t data)
{
    return i2c_write_byte(handle->i2c_port, handle->lcd_addr, LCD_REG_DATA, data);
}

esp_err_t jhd1313m3_init(const jhd1313m3_config_t *config, jhd1313m3_handle_t **handle)
{
    if (config == NULL || handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Allocate handle
    *handle = (jhd1313m3_handle_t *)malloc(sizeof(jhd1313m3_handle_t));
    if (*handle == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for handle");
        return ESP_ERR_NO_MEM;
    }

    (*handle)->i2c_port = config->i2c_port;
    (*handle)->lcd_addr = config->lcd_addr;
    (*handle)->rgb_addr = config->rgb_addr;

    // Wait for LCD to power up
    vTaskDelay(pdMS_TO_TICKS(50));

    // Initialize LCD (HD44780 compatible)
    // Function set: 8-bit mode, 2 lines, 5x8 dots
    lcd_send_command(*handle, LCD_CMD_FUNCTION_SET | LCD_8BIT_MODE | LCD_2LINE | LCD_5x8_DOTS);
    vTaskDelay(pdMS_TO_TICKS(5));

    // Display control: display on, cursor off, blink off
    (*handle)->display_control = LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF;
    lcd_send_command(*handle, LCD_CMD_DISPLAY_CTRL | (*handle)->display_control);

    // Entry mode: left to right, no shift
    (*handle)->display_mode = LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DEC;
    lcd_send_command(*handle, LCD_CMD_ENTRY_MODE | (*handle)->display_mode);

    // Clear display
    jhd1313m3_clear(*handle);

    // Initialize RGB backlight (PCA9633)
    // MODE1: normal mode
    i2c_write_byte((*handle)->i2c_port, (*handle)->rgb_addr, RGB_REG_MODE1, 0x00);
    // MODE2: DMBLNK = 0, INVRT = 0, OCH = 0, OUTDRV = 1, OUTNE = 00
    i2c_write_byte((*handle)->i2c_port, (*handle)->rgb_addr, RGB_REG_MODE2, 0x01);
    // LEDOUT: LED0-2 individual brightness and group dimming/blinking
    i2c_write_byte((*handle)->i2c_port, (*handle)->rgb_addr, RGB_REG_LEDOUT, 0xAA);

    // Set default white backlight
    jhd1313m3_set_rgb(*handle, 255, 255, 255);

    ESP_LOGI(TAG, "JHD1313M3 LCD initialized");
    return ESP_OK;
}

esp_err_t jhd1313m3_free(jhd1313m3_handle_t *handle)
{
    if (handle != NULL) {
        free(handle);
    }
    return ESP_OK;
}

esp_err_t jhd1313m3_clear(jhd1313m3_handle_t *handle)
{
    if (handle == NULL) return ESP_ERR_INVALID_ARG;

    esp_err_t ret = lcd_send_command(handle, LCD_CMD_CLEAR);
    vTaskDelay(pdMS_TO_TICKS(2)); // Clear takes longer
    return ret;
}

esp_err_t jhd1313m3_home(jhd1313m3_handle_t *handle)
{
    if (handle == NULL) return ESP_ERR_INVALID_ARG;

    esp_err_t ret = lcd_send_command(handle, LCD_CMD_HOME);
    vTaskDelay(pdMS_TO_TICKS(2)); // Home takes longer
    return ret;
}

esp_err_t jhd1313m3_set_cursor(jhd1313m3_handle_t *handle, uint8_t col, uint8_t row)
{
    if (handle == NULL) return ESP_ERR_INVALID_ARG;
    if (col >= LCD_COLS || row >= LCD_ROWS) return ESP_ERR_INVALID_ARG;

    // Row offsets for 16x2 display
    static const uint8_t row_offsets[] = {0x00, 0x40};

    return lcd_send_command(handle, LCD_CMD_SET_DDRAM_ADDR | (col + row_offsets[row]));
}

esp_err_t jhd1313m3_write_string(jhd1313m3_handle_t *handle, const char *str)
{
    if (handle == NULL || str == NULL) return ESP_ERR_INVALID_ARG;

    while (*str) {
        esp_err_t ret = lcd_send_data(handle, *str++);
        if (ret != ESP_OK) return ret;
    }
    return ESP_OK;
}

esp_err_t jhd1313m3_write_char(jhd1313m3_handle_t *handle, char c)
{
    if (handle == NULL) return ESP_ERR_INVALID_ARG;
    return lcd_send_data(handle, c);
}

esp_err_t jhd1313m3_set_rgb(jhd1313m3_handle_t *handle, uint8_t r, uint8_t g, uint8_t b)
{
    if (handle == NULL) return ESP_ERR_INVALID_ARG;

    esp_err_t ret;

    // Set PWM values for RGB (note: inverted logic for this controller)
    ret = i2c_write_byte(handle->i2c_port, handle->rgb_addr, RGB_REG_PWM_RED, r);
    if (ret != ESP_OK) return ret;

    ret = i2c_write_byte(handle->i2c_port, handle->rgb_addr, RGB_REG_PWM_GREEN, g);
    if (ret != ESP_OK) return ret;

    ret = i2c_write_byte(handle->i2c_port, handle->rgb_addr, RGB_REG_PWM_BLUE, b);

    return ret;
}

esp_err_t jhd1313m3_display(jhd1313m3_handle_t *handle, bool on)
{
    if (handle == NULL) return ESP_ERR_INVALID_ARG;

    if (on) {
        handle->display_control |= LCD_DISPLAY_ON;
    } else {
        handle->display_control &= ~LCD_DISPLAY_ON;
    }

    return lcd_send_command(handle, LCD_CMD_DISPLAY_CTRL | handle->display_control);
}

esp_err_t jhd1313m3_cursor(jhd1313m3_handle_t *handle, bool on)
{
    if (handle == NULL) return ESP_ERR_INVALID_ARG;

    if (on) {
        handle->display_control |= LCD_CURSOR_ON;
    } else {
        handle->display_control &= ~LCD_CURSOR_ON;
    }

    return lcd_send_command(handle, LCD_CMD_DISPLAY_CTRL | handle->display_control);
}

esp_err_t jhd1313m3_blink(jhd1313m3_handle_t *handle, bool on)
{
    if (handle == NULL) return ESP_ERR_INVALID_ARG;

    if (on) {
        handle->display_control |= LCD_BLINK_ON;
    } else {
        handle->display_control &= ~LCD_BLINK_ON;
    }

    return lcd_send_command(handle, LCD_CMD_DISPLAY_CTRL | handle->display_control);
}
