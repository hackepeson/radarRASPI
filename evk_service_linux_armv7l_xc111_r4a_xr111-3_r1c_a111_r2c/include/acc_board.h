// Copyright (c) Acconeer AB, 2015-2018
// All rights reserved

#ifndef ACC_BOARD_H_
#define ACC_BOARD_H_

#include <stdbool.h>
#include <stdint.h>

#include "acc_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Default SPI speed
 */
#define ACC_BOARD_DEFAULT_SPI_SPEED	5000000


/**
 * @brief Initialize board
 *
 * Should only be called from sensor_driver_init.
 *
 * @return Status
 */
extern acc_status_t acc_board_init(void);


/**
 * @brief Initialize the direction and level of host GPIOs
 *
 * Must be called after acc_board_init, otherwise the GPIO initialization will not succeed.
 *
 * @return Status
 */
extern acc_status_t acc_board_gpio_init(void);


/**
 * @brief Start sensor
 *
 * Setup in order to communicate with the specified sensor.
 * Must only be used by sensor driver (acc_sensor_driver_start_sensor()) to guarantee thread safety.
 *
 * @param[in] sensor The sensor to be started
 * @return Status
 */
extern acc_status_t acc_board_start_sensor(acc_sensor_t sensor);


/**
 * @brief Stop sensor
 *
 * Setup when not needing to communicate with the specified sensor.
 * Must only be used by sensor driver (acc_sensor_driver_stop_sensor()) to guarantee thread safety.
 *
 * @param[in] sensor The sensor to be stopped
 * @return Status
 */
extern acc_status_t acc_board_stop_sensor(acc_sensor_t sensor);


/**
 * @brief Retrieve SPI bus and CS numbers for a specific sensor
 *
 * @param[in] sensor The specific sensor
 * @param[out] bus The SPI bus
 * @param[out] cs The CS
 */
extern void acc_board_get_spi_bus_cs(acc_sensor_t sensor, uint_fast8_t *bus, uint_fast8_t *cs);


/**
 * @brief Custom chip select for SPI transfer
 *
 * To be called from sensor_driver_transfer.
 *
 * @param[in] sensor The specific sensor
 * @param[in] cs_assert Chip select or deselect
 * @return Status
 */
extern acc_status_t acc_board_chip_select(acc_sensor_t sensor, uint_fast8_t cs_assert);


/**
 * @brief Get information if the sensor interrupt pin is connected for the specified sensor
 *
 * @param[in] sensor Sensor to get interrupt information for
 * @return True if sensor interrupt is connected
 */
extern bool acc_board_is_sensor_interrupt_connected(acc_sensor_t sensor);


/**
 * @brief Get the current status of the sensor interrupt
 *
 * @param[in] sensor Sensor to get interrupt information for
 * @return True if sensor interrupt is active
 */
extern bool acc_board_is_sensor_interrupt_active(acc_sensor_t sensor);


/**
 * @brief Retrieves the number of sensors connected to the device
 *
 * @return The number of sensors
 */
extern acc_sensor_t acc_board_get_sensor_count(void);


/**
 * @brief Retrieves the reference frequency of the clock supplied from the board
 *
 * @return The reference frequency
 */
extern float acc_board_get_ref_freq(void);


/**
 * @brief Retrieve SPI bus speed
 *
 * @param[in] bus The SPI bus
 * @return SPI speed [Hz]
 */
extern uint32_t acc_board_get_spi_speed(uint_fast8_t bus);


/**
 * @brief Inform which reference frequency the system is using
 *
 * @param[in] ref_freq Reference frequency
 * @return Status
 */
extern acc_status_t acc_board_set_ref_freq(float ref_freq);


#ifdef __cplusplus
}
#endif

#endif
