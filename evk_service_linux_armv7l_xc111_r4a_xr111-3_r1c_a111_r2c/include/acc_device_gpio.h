// Copyright (c) Acconeer AB, 2016-2018
// All rights reserved

#ifndef ACC_DEVICE_GPIO_H_
#define ACC_DEVICE_GPIO_H_

#include <stdint.h>
#include <stdlib.h>

#include "acc_types.h"

#ifdef __cplusplus
extern "C" {
#endif


// These functions are to be used by drivers only, do not use them directly
extern acc_status_t	(*acc_device_gpio_init_func)(void);
extern acc_status_t	(*acc_device_gpio_set_initial_pull_func)(uint_fast8_t pin, uint_fast8_t level);
extern acc_status_t	(*acc_device_gpio_input_func)(uint_fast8_t pin);
extern acc_status_t	(*acc_device_gpio_read_func)(uint_fast8_t pin, uint_fast8_t *level);
extern acc_status_t	(*acc_device_gpio_write_func)(uint_fast8_t pin, uint_fast8_t level);


/**
 * @brief Initialize GPIO device
 *
 * @return Status
 */
extern acc_status_t acc_device_gpio_init(void);


/**
 * @brief Inform the driver of the pull up/down level for a GPIO pin after reset
 *
 * This does not change the pull level, but only informs the driver what pull level
 * the pin is configured to have.
 *
 * The GPIO pin numbering is decided by the GPIO driver.
 *
 * @param pin Pin number
 * @param level The pull level 0 or 1
 * @return Status
 */
extern acc_status_t acc_device_gpio_set_initial_pull(uint_fast8_t pin, uint_fast8_t level);


/**
 * @brief Set GPIO to input
 *
 * This function sets the direction of a GPIO to input.
 * The GPIO pin numbering is decided by the GPIO driver
 *
 * @param pin Pin to be set to input
 * @return Status
 */
extern acc_status_t acc_device_gpio_input(uint_fast8_t pin);


/**
 * @brief Read from GPIO
 *
 * The GPIO pin numbering is decided by the GPIO driver
 *
 * @param pin Pin to be read
 * @param level The pin level is returned here
 * @return Status
 */
extern acc_status_t acc_device_gpio_read(uint_fast8_t pin, uint_fast8_t *level);


/**
 * @brief Set GPIO output level
 *
 * This function sets a GPIO to output and the level to low or high.
 * The GPIO pin numbering is decided by the GPIO driver
 *
 * @param pin Pin to be written
 * @param level 0 to 1 to set pin low or high
 * @return Status
 */
extern acc_status_t acc_device_gpio_write(uint_fast8_t pin, uint_fast8_t level);

#ifdef __cplusplus
}
#endif

#endif
