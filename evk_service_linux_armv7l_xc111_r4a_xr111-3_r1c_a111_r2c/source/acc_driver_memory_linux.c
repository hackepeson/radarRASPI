// Copyright (c) Acconeer AB, 2017
// All rights reserved

/* Makes sure the function fseeko will be exposed in stdio.h and fsync in unistd.h. Must be defined before including stdio.h and unistd.h. */
#define _POSIX_C_SOURCE 200112L

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "acc_device_memory.h"
#include "acc_driver_memory_linux.h"
#include "acc_log.h"
#include "acc_types.h"

#define MODULE "driver_memory_linux"

#define MEMORY_FILE_NAME "memory.bin"

static FILE	*file = NULL;
static int	file_desc;


static acc_status_t acc_driver_memory_linux_init(void)
{
	file = fopen(MEMORY_FILE_NAME, "r+");
	if (file == NULL) {
		file = fopen(MEMORY_FILE_NAME, "w+");
		if (file == NULL) {
			ACC_LOG_ERROR("Failed to open %s. (%u) %s", MEMORY_FILE_NAME, errno, strerror(errno));
			return ACC_STATUS_FAILURE;
		}
	}

	file_desc = fileno(file);

	return ACC_STATUS_SUCCESS;
}


static acc_status_t acc_driver_memory_linux_write(uint32_t address, const void *buffer, size_t size)
{
	if (buffer == NULL) {
		return ACC_STATUS_BAD_PARAM;
	}

	if (file == NULL) {
		ACC_LOG_ERROR("Driver is not initialized");
		return ACC_STATUS_FAILURE;
	}

	if (fseeko(file, address, SEEK_SET) != 0) {
		ACC_LOG_ERROR("Failed to seek to address %u in file %s", (unsigned int)address, MEMORY_FILE_NAME);
		return ACC_STATUS_FAILURE;
	}

	if (fwrite(buffer, 1, size, file) != size) {
		ACC_LOG_ERROR("Failed to write to file %s", MEMORY_FILE_NAME);
		return ACC_STATUS_FAILURE;
	}

	fflush(file);
	fsync(file_desc);

	return ACC_STATUS_SUCCESS;
}


static acc_status_t acc_driver_memory_linux_read(uint32_t address, void *buffer, size_t size)
{
	size_t items_read;

	if (buffer == NULL) {
		return ACC_STATUS_BAD_PARAM;
	}

	if (file == NULL) {
		ACC_LOG_ERROR("Driver is not initialized");
		return ACC_STATUS_FAILURE;
	}

	if (fseeko(file, address, SEEK_SET) != 0) {
		ACC_LOG_ERROR("Failed to seek to address %u in file %s", (unsigned int)address, MEMORY_FILE_NAME);
		return ACC_STATUS_FAILURE;
	}

	items_read = fread(buffer, 1, size, file);
	if (items_read != size) {
		if (feof(file) != 0) {
			size_t pad_size = size - items_read;
			memset((char*)buffer + items_read, 0, pad_size);
		}
		else {
			ACC_LOG_ERROR("Failed to read from file %s", MEMORY_FILE_NAME);
			return ACC_STATUS_FAILURE;
		}
	}

	return ACC_STATUS_SUCCESS;
}


void acc_driver_memory_linux_register(void)
{
	acc_device_memory_init_func = acc_driver_memory_linux_init;
	acc_device_memory_write_func = acc_driver_memory_linux_write;
	acc_device_memory_read_func = acc_driver_memory_linux_read;
}
