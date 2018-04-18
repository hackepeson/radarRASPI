// Copyright (c) Acconeer AB, 2016-2018
// All rights reserved

#ifndef ACC_DEVICE_I2C_H_
#define ACC_DEVICE_I2C_H_

#include <stdint.h>
#include <stdlib.h>

#include "acc_types.h"

#ifdef __cplusplus
extern "C" {
#endif


// These functions are to be used by drivers only, do not use them directly
extern acc_status_t	(*acc_device_i2c_init_func)(void);
extern acc_status_t	(*acc_device_i2c_write_to_address_8_func)(uint8_t device_id, uint8_t address, const uint8_t *buffer, size_t buffer_size);
extern acc_status_t	(*acc_device_i2c_write_to_address_16_func)(uint8_t device_id, uint16_t address, const uint8_t *buffer, size_t buffer_size);
extern acc_status_t	(*acc_device_i2c_read_from_address_8_func)(uint8_t device_id, uint8_t address, uint8_t *buffer , size_t buffer_size);
extern acc_status_t	(*acc_device_i2c_read_from_address_16_func)(uint8_t device_id, uint16_t address, uint8_t *buffer , size_t buffer_size);
extern acc_status_t	(*acc_device_i2c_read_func)(uint8_t device_id, uint8_t *buffer, size_t buffer_size);


/**
 * @brief Initialize I2C device
 *
 * @return Status
 */
extern acc_status_t acc_device_i2c_init(void);


/**
 * @brief I2C write
 *
 * Write to specific device ID at specific 8-bit address.
 * Transfer including START and STOP.
 *
 * @param device_id The ID of the device to write to
 * @param address The 8-bit address to write to
 * @param buffer The data to be written
 * @param buffer_size The size of the buffer
 * @return Status
 */
extern acc_status_t acc_device_i2c_write_to_address_8(uint8_t device_id, uint8_t address, const uint8_t *buffer, size_t buffer_size);


/**
 * @brief I2C write
 *
 * Write to specific device ID at specific 16-bit address.
 * Transfer including START and STOP.
 *
 * @param device_id The ID of the device to write to
 * @param address The 16-bit address to write to
 * @param buffer The data to be written
 * @param buffer_size The size of the buffer
 * @return Status
 */
extern acc_status_t acc_device_i2c_write_to_address_16(uint8_t device_id, uint16_t address, const uint8_t *buffer, size_t buffer_size);


/**
 * @brief I2C read from specified 8-bit address
 *
 * Read from specific device ID from specific address.
 * Transfer including START and STOP.
 *
 * @param device_id The ID of the device to read from
 * @param address The 8-bit address to start reading from
 * @param buffer The result of the read is stored here
 * @param buffer_size The size of the buffer
 * @return Status
 */
extern acc_status_t acc_device_i2c_read_from_address_8(uint8_t device_id, uint8_t address, uint8_t *buffer, size_t buffer_size);


/**
 * @brief I2C read from specified 16-bit address
 *
 * Read from specific device ID from specific address.
 * Transfer including START and STOP.
 *
 * @param device_id The ID of the device to read from
 * @param address The 16-bit address to start reading from
 * @param buffer The result of the read is stored here
 * @param buffer_size The size of the buffer
 * @return Status
 */
extern acc_status_t acc_device_i2c_read_from_address_16(uint8_t device_id, uint16_t address, uint8_t *buffer, size_t buffer_size);


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
extern acc_status_t acc_device_i2c_read(uint8_t device_id, uint8_t *buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
