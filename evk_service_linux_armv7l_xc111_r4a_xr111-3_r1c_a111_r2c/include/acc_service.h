// Copyright (c) Acconeer AB, 2018
// All rights reserved

#ifndef ACC_SERVICE_H_
#define ACC_SERVICE_H_

#include <stdint.h>

#include "acc_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Return status for service methods
*/
typedef enum
{
	ACC_SERVICE_STATUS_OK,
	ACC_SERVICE_STATUS_FAILURE_UNSPECIFIED,
	ACC_SERVICE_STATUS_INVALID_CONFIGURATION,
	ACC_SERVICE_STATUS_INVALID_HANDLE,
	ACC_SERVICE_STATUS_INVALID_PARAMETER,
	ACC_SERVICE_STATUS_INVALID_STATE,
	ACC_SERVICE_STATUS_OUT_OF_MEMORY,
	ACC_SERVICE_STATUS_TIMEOUT,
	ACC_SERVICE_STATUS_MAX
} acc_service_status_enum_t;
typedef uint32_t acc_service_status_t;


/**
 * @brief Generic service configuration container
*/
struct acc_service_configuration;
typedef struct acc_service_configuration *acc_service_configuration_t;


/**
 * @brief Generic service handle
*/
struct acc_service_handle;
typedef struct acc_service_handle *acc_service_handle_t;


/**
 * @brief Return a string with a service status name
 * 
 * @param[in] status The service status to get the name for
 * @return Status string
 */
extern char *acc_service_status_name_get(acc_service_status_t status);


/**
 * @brief Create a service with the provided configuration
 *
 * Only one service may exist for a specific sensor at any given time and
 * invalid configurations will not allow for service creation.
 *
 * @param[in] configuration The service configuration to create a service with
 * @return Service handle, NULL if service was not possible to create
 */
extern acc_service_handle_t acc_service_create(acc_service_configuration_t configuration);


/**
 * @brief Activate the service associated with the provided handle
 *
 * @param[in] service_handle The service handle for the service to activate
 * @return Service status
 */
extern acc_service_status_t acc_service_activate(acc_service_handle_t service_handle);


/**
 * @brief Deactivate the service associated with the provided handle
 *
 * @param[in] service_handle The service handle for the service to deactivate
 * @return Service status
 */
extern acc_service_status_t acc_service_deactivate(acc_service_handle_t service_handle);


/**
 * @brief Destroy a service identified with the provided service handle
 *
 * Destroy the context of a service allowing another service to be created using the
 * same resources. The service handle reference is set to NULL after destruction.
 * 
 * @param[in] service_handle A reference to the service handle to destroy
 * @return Service status
 */
extern void acc_service_destroy(acc_service_handle_t *service_handle);


#ifdef __cplusplus
}
#endif

#endif
