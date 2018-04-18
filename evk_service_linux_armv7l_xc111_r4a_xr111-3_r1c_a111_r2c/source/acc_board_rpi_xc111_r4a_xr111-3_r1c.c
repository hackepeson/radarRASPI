// Copyright (c) Acconeer AB, 2017
// All rights reserved

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "acc_board.h"
#include "acc_device_gpio.h"
#include "acc_driver_gpio_linux_sysfs.h"
#include "acc_driver_i2c_linux.h"
#include "acc_driver_spi_linux_spidev.h"
#include "acc_log.h"
#include "acc_os.h"


/**
 * @brief The module name
 *
 * Must exist if acc_log.h is used.
 */
#define MODULE		"board_rpi_xc111_r4a_xr111-3_r1c"


/**
 * @brief List of supported hardware
 */
static char *supported_hardware[] = {"RPI XC111_R4A XR111-3_R1C A111_R2C", NULL };


/**
 * @brief The number of sensors available on the board
 */
#define SENSOR_COUNT	4

/**
 * @brief Host GPIO pin number (BCM)
 *
 * This GPIO should be connected to sensor 1 GPIO 5
 */
#define GPIO0_PIN	20

/**
 * @brief Host GPIO pin number (BCM)
 *
 * This GPIO should be connected to sensor 2 GPIO 5
 */
#define GPIO1_PIN	21

/**
 * @brief Host GPIO pin number (BCM)
 *
 * This GPIO should be connected to sensor 3 GPIO 5
 */
#define GPIO2_PIN	24

/**
 * @brief Host GPIO pin number (BCM)
 *
 * This GPIO should be connected to sensor 4 GPIO 5
 */
#define GPIO3_PIN	25

/**
 * @brief Host GPIO pin number (BCM)
 */
/**@{*/
#define RSTn_PIN	6
#define ENABLE_PIN	27

#define PMU_ENABLE_PIN	12
#define GLOREF_EN_PIN	13

#define CE_A_PIN	16
#define CE_B_PIN	19
/**@}*/

/**
 * @brief The reference frequency used by this board
 *
 * This assumes 24 MHz on XR111-3 R1C
 */
#define ACC_BOARD_REF_FREQ	24000000

/**
 * @brief The SPI speed of this board
 */
#define ACC_BOARD_SPI_SPEED	15000000

/**
 * @brief Host GPIO indices
 */
typedef enum {
	HOST_GPIO_GPIO0,
	HOST_GPIO_GPIO1,
	HOST_GPIO_GPIO2,
	HOST_GPIO_GPIO3,
	HOST_GPIO_RSTn,
	HOST_GPIO_ENABLE,
	HOST_GPIO_PMU_ENABLE,
	HOST_GPIO_GLOREF_EN,
	HOST_GPIO_CE_A,
	HOST_GPIO_CE_B,
	HOST_GPIO_MAX
} acc_board_host_gpio_enum_t;


/**
 * @brief Sensor states
 */
typedef enum {
	SENSOR_STATE_UNKNOWN,
	SENSOR_STATE_READY,
	SENSOR_STATE_BUSY
} sensor_state_t;


/**
 * @brief Vector with pin and pull information of host GPIOs
 */
acc_board_host_gpio_t acc_board_host_gpios[HOST_GPIO_MAX] = {
			{ .pin = GPIO0_PIN, .pull = 0 },
			{ .pin = GPIO1_PIN, .pull = 0 },
			{ .pin = GPIO2_PIN, .pull = 0 },
			{ .pin = GPIO3_PIN, .pull = 0 },
			{ .pin = RSTn_PIN, .pull = 1 },
			{ .pin = ENABLE_PIN, .pull = 0 },
			{ .pin = PMU_ENABLE_PIN, .pull = 0 },
			{ .pin = GLOREF_EN_PIN, .pull = 0 },
			{ .pin = CE_A_PIN, .pull = 0 },
			{ .pin = CE_B_PIN, .pull = 0 }
};


/**
 * @brief Vector with name information of host GPIOs
 */
char *acc_board_host_gpio_names[HOST_GPIO_MAX] = { "GPIO0", "GPIO1", "GPIO2", "GPIO3", "RSTn", "ENABLE", "PMU_ENABLE", "GLOREF_EN", "CE_A", "CE_B" };


/**
 * @brief Sensor state collection that keeps track of each sensor's current state
 */
static sensor_state_t sensor_state[SENSOR_COUNT];


/**
 * @brief File-local matrix with SPI bus and CS information for all sensors
 */
static uint_fast8_t sensor_spi_bus_cs[SENSOR_COUNT][2] = {
							{ 0, 0 },
							{ 0, 0 },
							{ 0, 0 },
							{ 0, 0 }
							 };


/**
 * @brief Set of acc_board_flag_t flags currently being set
 */
static uint32_t acc_board_flags;


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
void acc_board_set_flag(acc_board_flag_t flag)
{
	if (flag < ACC_BOARD_FLAG_MAX) {
		acc_board_flags |= 1 << flag;
	}
}


/**
 * @brief Get the combined status of all sensors
 *
 * @return False if any sensor is busy
 */
static bool acc_board_all_sensors_inactive(void)
{
	for (uint_fast8_t sensor_index = 0; sensor_index < SENSOR_COUNT; sensor_index++) {
		if (sensor_state[sensor_index] == SENSOR_STATE_BUSY) {
			return false;
		}
	}

	return true;
}


/**
 * @brief Initialize the direction and level of host GPIOs
 *
 * @return Status
 */
static acc_status_t acc_board_gpio_init(void)
{
	acc_status_t status;

	// special behaviour, do not init the GPIOs
	if ((acc_board_flags & 1 << ACC_BOARD_FLAG_GPIO_NO_INIT) ||
	    (acc_board_flags & 1 << ACC_BOARD_FLAG_GPIO_NO_GPIO)) {
		return ACC_STATUS_SUCCESS;
	}

	for (uint_fast8_t index = 0; index < HOST_GPIO_MAX; index++) {
		acc_device_gpio_set_initial_pull(acc_board_host_gpios[index].pin, acc_board_host_gpios[index].pull);
	}

	if (
		(status = acc_board_gpio_input(HOST_GPIO_GPIO0)) ||
		(status = acc_board_gpio_input(HOST_GPIO_GPIO1)) ||
		(status = acc_board_gpio_input(HOST_GPIO_GPIO2)) ||
		(status = acc_board_gpio_input(HOST_GPIO_GPIO3)) ||
		(status = acc_board_gpio_output(HOST_GPIO_RSTn, 0)) ||
		(status = acc_board_gpio_output(HOST_GPIO_ENABLE, 0)) ||
		(status = acc_board_gpio_output(HOST_GPIO_PMU_ENABLE, 0)) ||
		(status = acc_board_gpio_output(HOST_GPIO_GLOREF_EN, 0)) ||
		(status = acc_board_gpio_output(HOST_GPIO_CE_A, 0)) ||
		(status = acc_board_gpio_output(HOST_GPIO_CE_B, 0))
	) {
		ACC_LOG_ERROR("%s failed with %s", __func__, acc_log_status_name(status));
		return status;
	}

	return ACC_STATUS_SUCCESS;
}


/**
 * @brief Initialize board
 *
 * @return Status
 */
acc_status_t acc_board_init(void)
{
	acc_status_t		status;
	static bool		init_done = false;
	static acc_os_mutex_t	init_mutex;

	if (init_done) {
		return ACC_STATUS_SUCCESS;
	}

	acc_os_init();
	acc_os_mutex_init(&init_mutex);

	acc_os_mutex_lock(&init_mutex);
	if (init_done) {
		acc_os_mutex_unlock(&init_mutex);
		return ACC_STATUS_SUCCESS;
	}

	acc_driver_gpio_linux_sysfs_register(28);
	acc_driver_i2c_linux_register();
	acc_driver_spi_linux_spidev_register();

	if ((status = acc_board_gpio_init())) {
		acc_os_mutex_unlock(&init_mutex);
		return status;
	}

	for (uint_fast8_t sensor_index = 0; sensor_index < SENSOR_COUNT; sensor_index++) {
		sensor_state[sensor_index] = SENSOR_STATE_UNKNOWN;
	}

	init_done = true;
	acc_os_mutex_unlock(&init_mutex);

	return ACC_STATUS_SUCCESS;
}


/**
 * @brief Reset sensor
 *
 * Default setup when sensor is not active
 *
 * @return Status
 */
static acc_status_t acc_board_reset_sensor(void)
{
	acc_status_t status;

	status = acc_board_gpio_output(HOST_GPIO_RSTn, 0);
	if (status != ACC_STATUS_SUCCESS) {
		ACC_LOG_ERROR("Unable to activate RSTn");
		return status;
	}

	status = acc_board_gpio_output(HOST_GPIO_ENABLE, 0);
	if (status != ACC_STATUS_SUCCESS) {
		ACC_LOG_ERROR("Unable to deactivate ENABLE");
		return status;
	}

	status = acc_board_gpio_output(HOST_GPIO_PMU_ENABLE, 0);
	if (status != ACC_STATUS_SUCCESS) {
		ACC_LOG_ERROR("Unable to deactivate PMU_ENABLE");
		return status;
	}

	return ACC_STATUS_SUCCESS;
}


/**
 * @brief Start sensor
 *
 * Setup in order to communicate with the specified sensor.
 *
 * @param[in] sensor The sensor to be started
 * @return Status
 */
acc_status_t acc_board_start_sensor(acc_sensor_t sensor)
{
	acc_status_t status;

	if (sensor_state[sensor - 1] == SENSOR_STATE_BUSY) {
		ACC_LOG_ERROR("Sensor %u already active.", sensor);
		return ACC_STATUS_FAILURE;
	}

	if (acc_board_all_sensors_inactive()) {
		status = acc_board_gpio_output(HOST_GPIO_RSTn, 0);
		if (status != ACC_STATUS_SUCCESS) {
			ACC_LOG_ERROR("Unable to activate RSTn");
			acc_board_reset_sensor();
			return status;
		}

		status = acc_board_gpio_output(HOST_GPIO_PMU_ENABLE, 1);
		if (status != ACC_STATUS_SUCCESS) {
			ACC_LOG_ERROR("Unable to activate PMU_ENABLE");
			acc_board_reset_sensor();
			return status;
		}

		// Wait for PMU to stabilize
		acc_os_sleep_us(5000);

		status = acc_board_gpio_output(HOST_GPIO_ENABLE, 1);
		if (status != ACC_STATUS_SUCCESS) {
			ACC_LOG_ERROR("Unable to activate ENABLE");
			acc_board_reset_sensor();
			return status;
		}

		// Wait for Power On Reset
		acc_os_sleep_us(5000);

		status = acc_board_gpio_output(HOST_GPIO_RSTn, 1);
		if (status != ACC_STATUS_SUCCESS) {
			ACC_LOG_ERROR("Unable to deactivate RSTn");
			acc_board_reset_sensor();
			return status;
		}

		for (uint_fast8_t sensor_index = 0; sensor_index < SENSOR_COUNT; sensor_index++) {
			sensor_state[sensor_index] = SENSOR_STATE_READY;
		}
	}

	if (sensor_state[sensor - 1] != SENSOR_STATE_READY) {
		ACC_LOG_ERROR("Sensor has not been reset");
		return ACC_STATUS_FAILURE;
	}

	sensor_state[sensor - 1] = SENSOR_STATE_BUSY;

	return ACC_STATUS_SUCCESS;
}


/**
 * @brief Stop sensor
 *
 * Setup when not needing to communicate with the specified sensor.
 *
 * @param[in] sensor The sensor to be stopped
 * @return Status
 */
acc_status_t acc_board_stop_sensor(acc_sensor_t sensor)
{
	if (sensor_state[sensor - 1] != SENSOR_STATE_BUSY) {
		ACC_LOG_ERROR("Sensor %u already inactive.", sensor);
		return ACC_STATUS_FAILURE;
	}

	sensor_state[sensor - 1] = SENSOR_STATE_UNKNOWN;

	if (acc_board_all_sensors_inactive()) {
		return acc_board_reset_sensor();
	}

	return ACC_STATUS_SUCCESS;
}


/**
 * @brief Retrieve SPI bus and CS numbers for a specific sensor
 *
 * @param[in] sensor The specific sensor
 * @param[out] bus The SPI bus
 * @param[out] cs The CS
 */
void acc_board_get_spi_bus_cs(acc_sensor_t sensor, uint_fast8_t *bus, uint_fast8_t *cs)
{
	if ((sensor <= 0) || (sensor > SENSOR_COUNT)) {
		*bus = -1;
		*cs  = -1;
	} else {
		*bus = sensor_spi_bus_cs[sensor - 1][0];
		*cs  = sensor_spi_bus_cs[sensor - 1][1];
	}
}


/**
 * @brief Custom chip select for SPI transfer
 *
 * To be called from sensor_driver_transfer.
 *
 * @param[in] sensor The specific sensor
 * @param[in] cs_assert Chip select or deselect
 * @return Status
 */
acc_status_t acc_board_chip_select(acc_sensor_t sensor, uint_fast8_t cs_assert)
{
	acc_status_t status;

	if (cs_assert) {
		uint_fast8_t cea_val = (sensor == 1 || sensor == 2) ? 0 : 1;
		uint_fast8_t ceb_val = (sensor == 1 || sensor == 3) ? 0 : 1;

		if (
			(status = acc_board_gpio_output(HOST_GPIO_CE_A, cea_val)) ||
			(status = acc_board_gpio_output(HOST_GPIO_CE_B, ceb_val))
		) {
			ACC_LOG_ERROR("%s failed with %s", __func__, acc_log_status_name(status));
			return status;
		}
	}

	return ACC_STATUS_SUCCESS;
}


/**
 * @brief Set host GPIO to input
 *
 * This function sets the direction of a host GPIO to input.
 * GPIO parameter is not a pin number but GPIO index (0-X).
 *
 * @param[in] gpio Host GPIO to be set to input
 * @return Status
 */
acc_status_t acc_board_gpio_input(uint_fast8_t gpio)
{
	acc_status_t status;

	if (gpio >= HOST_GPIO_MAX) {
		ACC_LOG_ERROR("GPIO %u is not a valid GPIO", gpio);
		return ACC_STATUS_BAD_PARAM;
	}

	status = acc_device_gpio_input(acc_board_host_gpios[gpio].pin);
	if (status != ACC_STATUS_SUCCESS) {
		ACC_LOG_ERROR("%s failed with %s", __func__, acc_log_status_name(status));
	}

	return status;
}


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
acc_status_t acc_board_gpio_output(uint_fast8_t gpio, uint_fast8_t level)
{
	acc_status_t	status;

	if (gpio >= HOST_GPIO_MAX) {
		ACC_LOG_ERROR("GPIO %u is not a valid GPIO", gpio);
		return ACC_STATUS_BAD_PARAM;
	}

	status = acc_device_gpio_write(acc_board_host_gpios[gpio].pin, level);
	if (status != ACC_STATUS_SUCCESS) {
		ACC_LOG_ERROR("%s failed with %s", __func__, acc_log_status_name(status));
	}

	return status;
}


/**
 * @brief Read from host GPIO
 *
 * GPIO parameter is not a pin number but GPIO index (0-X).
 *
 * @param[in] gpio Host GPIO to read from
 * @param[out] level The level is returned here
 * @return Status
 */
acc_status_t acc_board_gpio_read(uint_fast8_t gpio, uint_fast8_t *level)
{
	acc_status_t	status;

	if (gpio >= HOST_GPIO_MAX) {
		ACC_LOG_ERROR("GPIO %u is not a valid GPIO", gpio);
		return ACC_STATUS_BAD_PARAM;
	}

	status = acc_device_gpio_read(acc_board_host_gpios[gpio].pin, level);
	if (status != ACC_STATUS_SUCCESS) {
		ACC_LOG_ERROR("%s failed with %s", __func__, acc_log_status_name(status));
	}

	return status;
}


/**
 * @brief Retrieves the number of sensors connected to the device
 *
 * @return The number of sensors
 */
acc_sensor_t acc_board_get_sensor_count(void)
{
	return SENSOR_COUNT;
}


/**
 * @brief Retrieves the number of host GPIOs available on the device
 *
 * @return The number of available host GPIOs
 */
uint_fast8_t acc_board_get_host_gpio_count(void)
{
	if (acc_board_flags & 1 << ACC_BOARD_FLAG_GPIO_NO_GPIO) {
		return 0;
	}

	return HOST_GPIO_MAX;
}


/**
 * @brief Retrieves the reference frequency of the clock supplied from the board
 *
 * @return The reference frequency
 */
float acc_board_get_ref_freq(void)
{
	return ACC_BOARD_REF_FREQ;
}


/**
 * @brief Retrieve SPI bus speed
 *
 * @param[in] bus The SPI bus
 * @return SPI speed [Hz]
 */
uint32_t acc_board_get_spi_speed(uint_fast8_t bus)
{
	ACC_UNUSED(bus);

	return ACC_BOARD_SPI_SPEED;
}


/**
 * @brief Inform which reference frequency the system is using
 *
 * @param[in] ref_freq Reference frequency
 * @return Status
 */
acc_status_t acc_board_set_ref_freq(float ref_freq)
{
	ACC_UNUSED(ref_freq);

	return ACC_STATUS_UNSUPPORTED;
}


/**
 * @brief Return a list of supported hardware
 *
 * @return List of supported hardware as strings
 */
char **acc_board_get_supported_hardware(void)
{
	return supported_hardware;
}
