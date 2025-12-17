/**
 * @file jhd1313m3.h
 * @brief JHD1313M3 RGB LCD Display Driver for ESP-IDF
 *
 * Grove RGB Backlight LCD (16x2 character display with RGB backlight)
 * - LCD Controller: JHD1313M3 (HD44780 compatible) @ I2C 0x3E
 * - RGB Controller: PCA9633 @ I2C 0x62
 */

#ifndef JHD1313M3_H
#define JHD1313M3_H

#include <stdint.h>
#include "driver/i2c.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2C Addresses
#define JHD1313M3_LCD_ADDR      0x3E  // LCD text controller
#define JHD1313M3_RGB_ADDR      0x62  // RGB backlight controller

// LCD Commands (HD44780 compatible)
#define LCD_CMD_CLEAR           0x01
#define LCD_CMD_HOME            0x02
#define LCD_CMD_ENTRY_MODE      0x04
#define LCD_CMD_DISPLAY_CTRL    0x08
#define LCD_CMD_SHIFT           0x10
#define LCD_CMD_FUNCTION_SET    0x20
#define LCD_CMD_SET_CGRAM_ADDR  0x40
#define LCD_CMD_SET_DDRAM_ADDR  0x80

// Entry Mode flags
#define LCD_ENTRY_RIGHT         0x00
#define LCD_ENTRY_LEFT          0x02
#define LCD_ENTRY_SHIFT_INC     0x01
#define LCD_ENTRY_SHIFT_DEC     0x00

// Display Control flags
#define LCD_DISPLAY_ON          0x04
#define LCD_DISPLAY_OFF         0x00
#define LCD_CURSOR_ON           0x02
#define LCD_CURSOR_OFF          0x00
#define LCD_BLINK_ON            0x01
#define LCD_BLINK_OFF           0x00

// Function Set flags
#define LCD_8BIT_MODE           0x10
#define LCD_4BIT_MODE           0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10_DOTS           0x04
#define LCD_5x8_DOTS            0x00

// RGB registers (PCA9633)
#define RGB_REG_MODE1           0x00
#define RGB_REG_MODE2           0x01
#define RGB_REG_PWM_RED         0x02
#define RGB_REG_PWM_GREEN       0x03
#define RGB_REG_PWM_BLUE        0x04
#define RGB_REG_LEDOUT          0x08

// Display size
#define LCD_COLS                16
#define LCD_ROWS                2

/**
 * @brief JHD1313M3 configuration structure
 */
typedef struct {
    i2c_port_t i2c_port;        // I2C port number
    uint8_t lcd_addr;            // LCD I2C address (default 0x3E)
    uint8_t rgb_addr;            // RGB I2C address (default 0x62)
} jhd1313m3_config_t;

/**
 * @brief JHD1313M3 device handle
 */
typedef struct {
    i2c_port_t i2c_port;
    uint8_t lcd_addr;
    uint8_t rgb_addr;
    uint8_t display_control;
    uint8_t display_mode;
} jhd1313m3_handle_t;

/**
 * @brief Initialize JHD1313M3 LCD display
 *
 * @param config Configuration structure
 * @param handle Pointer to device handle (output)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t jhd1313m3_init(const jhd1313m3_config_t *config, jhd1313m3_handle_t **handle);

/**
 * @brief Deinitialize and free JHD1313M3 device
 *
 * @param handle Device handle
 * @return esp_err_t ESP_OK on success
 */
esp_err_t jhd1313m3_free(jhd1313m3_handle_t *handle);

/**
 * @brief Clear display and return cursor to home
 *
 * @param handle Device handle
 * @return esp_err_t ESP_OK on success
 */
esp_err_t jhd1313m3_clear(jhd1313m3_handle_t *handle);

/**
 * @brief Return cursor to home position (0,0)
 *
 * @param handle Device handle
 * @return esp_err_t ESP_OK on success
 */
esp_err_t jhd1313m3_home(jhd1313m3_handle_t *handle);

/**
 * @brief Set cursor position
 *
 * @param handle Device handle
 * @param col Column (0-15)
 * @param row Row (0-1)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t jhd1313m3_set_cursor(jhd1313m3_handle_t *handle, uint8_t col, uint8_t row);

/**
 * @brief Write string to LCD
 *
 * @param handle Device handle
 * @param str String to write
 * @return esp_err_t ESP_OK on success
 */
esp_err_t jhd1313m3_write_string(jhd1313m3_handle_t *handle, const char *str);

/**
 * @brief Write character to LCD
 *
 * @param handle Device handle
 * @param c Character to write
 * @return esp_err_t ESP_OK on success
 */
esp_err_t jhd1313m3_write_char(jhd1313m3_handle_t *handle, char c);

/**
 * @brief Set RGB backlight color
 *
 * @param handle Device handle
 * @param r Red (0-255)
 * @param g Green (0-255)
 * @param b Blue (0-255)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t jhd1313m3_set_rgb(jhd1313m3_handle_t *handle, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Turn display on/off
 *
 * @param handle Device handle
 * @param on true = on, false = off
 * @return esp_err_t ESP_OK on success
 */
esp_err_t jhd1313m3_display(jhd1313m3_handle_t *handle, bool on);

/**
 * @brief Turn cursor on/off
 *
 * @param handle Device handle
 * @param on true = on, false = off
 * @return esp_err_t ESP_OK on success
 */
esp_err_t jhd1313m3_cursor(jhd1313m3_handle_t *handle, bool on);

/**
 * @brief Turn cursor blink on/off
 *
 * @param handle Device handle
 * @param on true = on, false = off
 * @return esp_err_t ESP_OK on success
 */
esp_err_t jhd1313m3_blink(jhd1313m3_handle_t *handle, bool on);

#ifdef __cplusplus
}
#endif

#endif // JHD1313M3_H
