// Copyright (c) Acconeer AB, 2015-2018
// All rights reserved

#ifndef ACC_BOARD_H_
#define ACC_BOARD_H_

#include <stdint.h>

#include "acc_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief This is the sensors' gpio count
 *
 * Must be 6.
 */
#define ACC_BOARD_SENSOR_GPIO_COUNT	6


/**
 * @brief Default SPI speed
 */
#define ACC_BOARD_DEFAULT_SPI_SPEED	5000000


/**
 * @brief Board flags for special behaviour
 */
typedef enum {
	ACC_BOARD_FLAG_GPIO_NO_INIT,		// configure GPIOs but do not set direction and level at startup
	ACC_BOARD_FLAG_GPIO_NO_GPIO,		// report zero GPIOs on the board
	ACC_BOARD_FLAG_MAX			// marker for highest flag number
} acc_board_flag_t;


/**
 * @brief Host GPIO information type
 */
typedef struct {
	uint_fast8_t	pin;
	uint_fast8_t	pull;
} acc_board_host_gpio_t;


/**
 * @brief Vector with dir, pull and pin information of host GPIOs
 */
extern acc_board_host_gpio_t acc_board_host_gpios[];


/**
 * @brief Vector with name information of host GPIOs
 */
extern char *acc_board_host_gpio_names[];


/**
 * @brief Set special flag, and depending on the flag it must be done before calling any init function
 *
 * Sets a special flag in the board module. The effect of doing it depends on the specific flag. Setting
 * multiple flags is done by calling this function multiple times.
 *
 * For special GPIO handling, this function must be called before any call to acc_board_init().
 *
 * Applications are allowed to include acc_board.h to call this function. Since most flags are related to
 * debugging behaviours this will only be used for special applications.
 *
 * There is no way to unset a flag, so if you don't want it, don't set it.
 *
 * @param[in]  flag The specific flag to be set
 */
extern void acc_board_set_flag(acc_board_flag_t flag);


/**
 * @brief Initialize board
 *
 * Should only be called from sensor_driver_init.
 *
 * @return Status
 */
extern acc_status_t acc_board_init(void);


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
 * @brief Set host GPIO to input
 *
 * This function sets the direction of a host GPIO to input.
 * GPIO parameter is not a pin number but GPIO index (0-X).
 *
 * @param[in] gpio Host GPIO to be set to input
 * @return Status
 */
extern acc_status_t acc_board_gpio_input(uint_fast8_t gpio);


/**
 * @brief Set host GPIO output level
 *
 * This function sets a host GPIO to output and the level to low or high.
 * GPIO parameter is not a pin number but GPIO index (0-X).
 *
 * @param[in] gpio Host GPIO to be set
 * @param[in] level 0 to 1 to set pin low or high
 * @return Status
 */
extern acc_status_t acc_board_gpio_output(uint_fast8_t gpio, uint_fast8_t level);


/**
 * @brief Read from host GPIO
 *
 * GPIO parameter is not a pin number but GPIO index (0-X).
 *
 * @param[in] gpio Host GPIO to read from
 * @param[out] level The level is returned here
 * @return Status
 */
extern acc_status_t acc_board_gpio_read(uint_fast8_t gpio, uint_fast8_t *level);


/**
 * @brief Retrieves the number of sensors connected to the device
 *
 * @return The number of sensors
 */
extern acc_sensor_t acc_board_get_sensor_count(void);


/**
 * @brief Retrieves the number of host GPIOs available on the device
 *
 * @return The number of available host GPIOs
 */
extern uint_fast8_t acc_board_get_host_gpio_count(void);


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


/**
 * @brief Return a list of supported hardware
 *
 * @return List of supported hardware as strings
 */
extern char **acc_board_get_supported_hardware(void);


#ifdef __cplusplus
}
#endif

#endif
