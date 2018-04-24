// Copyright (c) Acconeer AB, 2016-2018
// All rights reserved

#ifndef ACC_DRIVER_GPIO_LINUX_SYSFS_H_
#define ACC_DRIVER_GPIO_LINUX_SYSFS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Request driver to register with appropriate device(s)
 *
 * @param pin_count The maximum number of pins supported
 */
extern void acc_driver_gpio_linux_sysfs_register(uint_fast16_t pin_count);

#ifdef __cplusplus
}
#endif

#endif
