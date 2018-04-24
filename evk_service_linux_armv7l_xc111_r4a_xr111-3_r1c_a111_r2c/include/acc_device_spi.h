// Copyright (c) Acconeer AB, 2016-2018
// All rights reserved

#ifndef ACC_DEVICE_SPI_H_
#define ACC_DEVICE_SPI_H_

#include <stdint.h>
#include <stdlib.h>

#include "acc_types.h"

#ifdef __cplusplus
extern "C" {
#endif


#define ACC_DEVICE_SPI_BUS_MAX	2


// These functions are to be used by drivers only, do not use them directly
extern acc_status_t	(*acc_device_spi_init_func)(void);
extern size_t		(*acc_device_spi_get_max_transfer_size_func)(void);
extern acc_status_t	(*acc_device_spi_transfer_func)(uint_fast8_t bus, uint_fast8_t device, uint32_t speed, uint8_t *buffer, size_t buffer_size);


/**
 * @brief Initialize SPI driver
 *
 * @return Status
 */
extern acc_status_t acc_device_spi_init(void);


/**
 * @brief Reserve SPI bus
 *
 * @param bus The SPI bus to reserve
 * @return Status
 */
extern acc_status_t acc_device_spi_lock(uint_fast8_t bus);


/**
 * @brief Release SPI bus
 *
 * @param bus The SPI bus to release
 * @return Status
 */
extern acc_status_t acc_device_spi_unlock(uint_fast8_t bus);


/**
 * @brief Return maximum allowed size of one SPI transfer
 *
 * @return Maximum allowed transfer size in bytes, or zero if unknown
 */
extern size_t acc_device_spi_get_max_transfer_size(void);


/**
 * @brief Data transfer (SPI)
 *
 * TODO: Note, max transfer limit is between 2048 B and 4096 B.
 *
 * @param bus The SPI bus to transfer to/from
 * @param device The SPI device to transfer to/from
 * @param speed SPI transfer speed in bps
 * @param buffer The data to be transferred
 * @param buffer_size The size of the buffer in bytes
 * @return Status
 */
extern acc_status_t acc_device_spi_transfer(
		uint_fast8_t	bus,
		uint_fast8_t	device,
		uint32_t	speed,
		uint8_t		*buffer,
		size_t		buffer_size);

#ifdef __cplusplus
}
#endif

#endif
