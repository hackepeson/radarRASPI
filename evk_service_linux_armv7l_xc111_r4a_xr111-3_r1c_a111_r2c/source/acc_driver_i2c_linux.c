// Copyright (c) Acconeer AB, 2016-2017
// All rights reserved

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "acc_driver_i2c_linux.h"
#include "acc_device_i2c.h"
#include "acc_log.h"
#include "acc_types.h"


/**
 * @brief The module name
 */
#define MODULE		"driver_i2c_linux"

/**
 * @brief The path to the I2C device file to the Kernel
 */
#define I2C_PATH	"/dev/i2c-1"


/**
 * @brief File descriptor of I2C device file to Kernel
 */
static int		i2c_fd = -1;

/**
 * @brief Mutex to protect I2C transfers
 */
static pthread_mutex_t	i2c_mutex = PTHREAD_MUTEX_INITIALIZER;


/**
 * @brief Internal I2C read
 *
 * Does a read() on i2c_fd. Assumes that a device ID and optionally an address have already been set.
 *
 * @param buffer The result of the read is stored here
 * @param buffer_size The size of the buffer
 * @return Status
 */
static acc_status_t internal_read(uint8_t *buffer, size_t buffer_size)
{
	ssize_t bytes_read;
	if ((bytes_read = read(i2c_fd, buffer, buffer_size)) < 0) {
		ACC_LOG_ERROR("Could not read from i2c device: (%u) %s", errno, strerror(errno));
		return ACC_STATUS_FAILURE;
	}
	if ((size_t)bytes_read != buffer_size) {
		ACC_LOG_ERROR("Number of bytes read was %u, but should be %u", bytes_read, buffer_size);
		return ACC_STATUS_FAILURE;
	}

	return ACC_STATUS_SUCCESS;
}


/**
 * @brief Initialize I2C driver
 *
 * @return Status
 */
static acc_status_t acc_driver_i2c_linux_init(void)
{
	static pthread_mutex_t	init_mutex = PTHREAD_MUTEX_INITIALIZER;
	static bool		init_done;

	pthread_mutex_lock(&init_mutex);
	if (init_done) {
		pthread_mutex_unlock(&init_mutex);
		return ACC_STATUS_SUCCESS;
	}

	if ((i2c_fd = open(I2C_PATH, O_RDWR)) < 0) {
		ACC_LOG_FATAL("Unable to open i2c connection: %s", strerror(errno));
		pthread_mutex_unlock(&init_mutex);
		return ACC_STATUS_FAILURE;
	}

	init_done = true;
	pthread_mutex_unlock(&init_mutex);

	return ACC_STATUS_SUCCESS;
}


/**
 * @brief I2C write
 *
 * Write to specific device ID at specific address.
 * Transfer including START and STOP.
 *
 * @param[in] device_id The ID of the device to write to
 * @param[in] address The address to write to
 * @param[in] address_size Number of bytes of address data
 * @param[in] buffer The data to be written
 * @param[in] buffer_size The size of the buffer
 * @return Status
 */
static acc_status_t acc_driver_i2c_linux_write_to_address_internal(uint8_t device_id, uint16_t address, uint_fast8_t address_size, const uint8_t *buffer, size_t buffer_size)
{
	uint8_t write_data[address_size + buffer_size];
	ssize_t bytes_written;

	if (address_size == 1) {
		write_data[0] = address;
	} else if (address_size == 2) {
		write_data[0] = (address >> 8) & 0xff;
		write_data[1] = address & 0xff;
	} else {
		return ACC_STATUS_BAD_PARAM;
	}
	memcpy(&write_data[address_size], buffer, buffer_size);

	pthread_mutex_lock(&i2c_mutex);

	if (ioctl(i2c_fd, 0x0703, device_id) < 0) {
		ACC_LOG_ERROR("Could not set i2c slave device ID %u", device_id);
		pthread_mutex_unlock(&i2c_mutex);
		return ACC_STATUS_FAILURE;
	}

	if ((bytes_written = write(i2c_fd, write_data, sizeof(write_data))) < 0) {
		ACC_LOG_ERROR("Could not write to i2c device %u: (%u) %s", device_id, errno, strerror(errno));
		pthread_mutex_unlock(&i2c_mutex);
		return ACC_STATUS_FAILURE;
	}
	if ((size_t)bytes_written != sizeof(write_data)) {
		ACC_LOG_ERROR("Number of bytes written was %u, but should be %u", (unsigned int)bytes_written, (unsigned int)sizeof(write_data));
		pthread_mutex_unlock(&i2c_mutex);
		return ACC_STATUS_FAILURE;
	}

	pthread_mutex_unlock(&i2c_mutex);

	return ACC_STATUS_SUCCESS;
}


/**
 * @brief I2C write
 *
 * Write to specific device ID at specific 8-bit address.
 * Transfer including START and STOP.
 *
 * @param[in] device_id The ID of the device to write to
 * @param[in] address The 8-bit address to write to
 * @param[in] buffer The data to be written
 * @param[in] buffer_size The size of the buffer
 * @return Status
 */
static acc_status_t acc_driver_i2c_linux_write_to_address_8(uint8_t device_id, uint8_t address, const uint8_t *buffer, size_t buffer_size)
{
	return acc_driver_i2c_linux_write_to_address_internal(device_id, address, 1, buffer, buffer_size);
}


/**
 * @brief I2C write
 *
 * Write to specific device ID at specific 16-bit address.
 * Transfer including START and STOP.
 *
 * @param[in] device_id The ID of the device to write to
 * @param[in] address The 16-bit address to write to
 * @param[in] buffer The data to be written
 * @param[in] buffer_size The size of the buffer
 * @return Status
 */
static acc_status_t acc_driver_i2c_linux_write_to_address_16(uint8_t device_id, uint16_t address, const uint8_t *buffer, size_t buffer_size)
{
	return acc_driver_i2c_linux_write_to_address_internal(device_id, address, 2, buffer, buffer_size);
}


/**
 * @brief I2C read from specified address
 *
 * Read from specific device ID from specific address.
 * Transfer including START and STOP.
 *
 * @param[in] device_id The ID of the device to read from
 * @param[in] address The address to start reading from
 * @param[in] address_size Number of bytes of address data
 * @param[out] buffer The result of the read is stored here
 * @param[in] buffer_size The size of the buffer
 * @return Status
 */
static acc_status_t acc_driver_i2c_linux_read_from_address_internal(uint8_t device_id, uint16_t address, uint_fast8_t address_size, uint8_t *buffer , size_t buffer_size)
{
	acc_status_t	status;
	uint8_t		write_data[address_size];
	ssize_t		bytes_written;

	if (address_size == 1) {
		write_data[0] = address;
	} else if (address_size == 2) {
		write_data[0] = (address >> 8) & 0xff;
		write_data[1] = address & 0xff;
	} else {
		return ACC_STATUS_BAD_PARAM;
	}

	pthread_mutex_lock(&i2c_mutex);

	if (ioctl(i2c_fd, 0x0703, device_id) < 0) {
		ACC_LOG_ERROR("Could not set i2c slave device ID %u", device_id);
		pthread_mutex_unlock(&i2c_mutex);
		return ACC_STATUS_FAILURE;
	}

	if ((bytes_written = write(i2c_fd, write_data, address_size)) < 0) {
		ACC_LOG_ERROR("Could not write to i2c device %u: (%u) %s", device_id, errno, strerror(errno));
		pthread_mutex_unlock(&i2c_mutex);
		return ACC_STATUS_FAILURE;
	}
	if (bytes_written != address_size) {
		ACC_LOG_ERROR("Number of bytes written was %u, but should be %u", (unsigned int)bytes_written, (unsigned int)address_size);
		pthread_mutex_unlock(&i2c_mutex);
		return ACC_STATUS_FAILURE;
	}

	status = internal_read(buffer, buffer_size);
	if (status != ACC_STATUS_SUCCESS) {
		pthread_mutex_unlock(&i2c_mutex);
		return status;
	}

	pthread_mutex_unlock(&i2c_mutex);

	return ACC_STATUS_SUCCESS;
}


/**
 * @brief I2C read from specified 8-bit address
 *
 * Read from specific device ID from specific 8-bit address.
 * Transfer including START and STOP.
 *
 * @param[in] device_id The ID of the device to read from
 * @param[in] address The 8-bit address to start reading from
 * @param[out] buffer The result of the read is stored here
 * @param[in] buffer_size The size of the buffer
 * @return Status
 */
static acc_status_t acc_driver_i2c_linux_read_from_address_8(uint8_t device_id, uint8_t address, uint8_t *buffer , size_t buffer_size)
{
	return acc_driver_i2c_linux_read_from_address_internal(device_id, address, 1, buffer, buffer_size);
}


/**
 * @brief I2C read from specified 16-bit address
 *
 * Read from specific device ID from specific 16-bit address.
 * Transfer including START and STOP.
 *
 * @param[in] device_id The ID of the device to read from
 * @param[in] address The 16-bit address to start reading from
 * @param[out] buffer The result of the read is stored here
 * @param[in] buffer_size The size of the buffer
 * @return Status
 */
static acc_status_t acc_driver_i2c_linux_read_from_address_16(uint8_t device_id, uint16_t address, uint8_t *buffer , size_t buffer_size)
{
	return acc_driver_i2c_linux_read_from_address_internal(device_id, address, 2, buffer, buffer_size);
}


/**
 * @brief I2C read from last read/written address
 *
 * Read from specific device ID at last read/written address. Depending on device, the address might have
 * been automatically incremented since last read/write.
 * Transfer including START and STOP.
 *
 * @param device_id The ID of the device to read from
 * @param buffer The result of the read is stored here
 * @param buffer_size The size of the buffer
 * @return Status
 */
static acc_status_t acc_driver_i2c_linux_read(uint8_t device_id, uint8_t *buffer, size_t buffer_size)
{
	pthread_mutex_lock(&i2c_mutex);

	acc_status_t status;

	if (ioctl(i2c_fd, 0x0703, device_id) < 0) {
		ACC_LOG_ERROR("Could not set i2c device ID %u: (%u) %s", device_id, errno, strerror(errno));
		pthread_mutex_unlock(&i2c_mutex);
		return ACC_STATUS_FAILURE;
	}

	status = internal_read(buffer, buffer_size);
	if (status != ACC_STATUS_SUCCESS) {
		pthread_mutex_unlock(&i2c_mutex);
		return status;
	}

	pthread_mutex_unlock(&i2c_mutex);

	return ACC_STATUS_SUCCESS;
}


/**
 * @brief Request driver to register with appropriate device(s)
 */
void acc_driver_i2c_linux_register(void)
{
	acc_device_i2c_init_func			= acc_driver_i2c_linux_init;
	acc_device_i2c_write_to_address_8_func		= acc_driver_i2c_linux_write_to_address_8;
	acc_device_i2c_write_to_address_16_func		= acc_driver_i2c_linux_write_to_address_16;
	acc_device_i2c_read_from_address_8_func		= acc_driver_i2c_linux_read_from_address_8;
	acc_device_i2c_read_from_address_16_func	= acc_driver_i2c_linux_read_from_address_16;
	acc_device_i2c_read_func			= acc_driver_i2c_linux_read;
}
