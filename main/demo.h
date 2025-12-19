/**
 * @file demo.h
 * @brief Demo mode for testing Leveller hardware modules
 */

#ifndef DEMO_H
#define DEMO_H

#include <stdint.h>
#include "hal/i2c_types.h"

/**
 * @brief Initialize demo module
 * @param i2c_port I2C port number to use
 */
void demo_init(i2c_port_t i2c_port);

/**
 * @brief Run RGB color rotation demo
 * @note This function runs indefinitely
 */
void demo_run_rgb_color_rotation(void);

#endif // DEMO_H
