// Copyright (c) Acconeer AB, 2015-2018
// All rights reserved

#ifndef ACC_TYPES_H_
#define ACC_TYPES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define ACC_UNUSED(VAR)	(void)(VAR)


/**
 * @brief Identifies a remote computer in a multi node environment, 0 is the local node
*/
typedef uint16_t	acc_node_t;

/**
 * @brief Uniquely identifies concurrent requests from the Acconeer API used for matching responses
*/
typedef uint32_t	acc_request_id_t;

/**
 * @brief Identifies a sensor as seen from the current node, first sensor is 1
*/
typedef uint32_t	acc_sensor_t;

/**
 * @brief Return status used from most Acconeer functions
*/
typedef enum {
	ACC_STATUS_SUCCESS = 0,
	ACC_STATUS_BAD_PARAM,
	ACC_STATUS_INVALID_DRIVER,
	ACC_STATUS_INVALID_NODE,
	ACC_STATUS_INVALID_SENSOR,
	ACC_STATUS_INVALID_REQUEST_ID,
	ACC_STATUS_FAILURE,
	ACC_STATUS_NO_RESPONSE,
	ACC_STATUS_OUT_OF_MEMORY,
	ACC_STATUS_TIMEOUT,
	ACC_STATUS_UNSUPPORTED,
	ACC_STATUS_MAX
} acc_status_enum_t;
typedef uint32_t acc_status_t;

typedef void (acc_request_callback_t)(acc_request_id_t id, void *response, void *callback_data);
#ifdef __cplusplus
}
#endif

#endif
