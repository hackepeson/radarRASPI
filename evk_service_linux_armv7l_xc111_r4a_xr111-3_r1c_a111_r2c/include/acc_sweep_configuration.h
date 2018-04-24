// Copyright (c) Acconeer AB, 2018
// All rights reserved

#ifndef ACC_SWEEP_CONFIGURATION_H_
#define ACC_SWEEP_CONFIGURATION_H_

#include <stdint.h>

#include "acc_definitions.h"

#include "acc_service.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Power save mode
 *
 * Each power save mode corresponds to a how much of the sensor hardware is shutdown
 * between sweeps. Mode A means that the whole sensor is shutdown between sweeps while
 * mode D means that the sensor is in its active state all the time.
 * For each power save mode there will be a limit in the achievable update rate. Mode A
 * will have the lowest update rate limit but also consumes the least amount of power for
 * low update rates.
 * The update rate limits also depend on integration and range settings so for each
 * scenario it is up to the user to find the best possible compromise between update rate
 * and power consumption.
*/
typedef enum
{
	ACC_SWEEP_CONFIGURATION_POWER_SAVE_MODE_A,
	ACC_SWEEP_CONFIGURATION_POWER_SAVE_MODE_B,
	ACC_SWEEP_CONFIGURATION_POWER_SAVE_MODE_C,
	ACC_SWEEP_CONFIGURATION_POWER_SAVE_MODE_D,
	ACC_SWEEP_CONFIGURATION_POWER_SAVE_MODE_COUNT
} acc_sweep_configuration_power_save_mode_enum_t;
typedef uint32_t acc_sweep_configuration_power_save_mode_t;


/**
 * @brief Sweep configuration container
*/
struct acc_sweep_configuration;
typedef struct acc_sweep_configuration *acc_sweep_configuration_t;


/**
 * @brief Retreive a sweep configuration from a service configuration
 *
 * @param[in] service_configuration The service configuration to get a sweep configuration from
 * @return Sweep configuration, NULL if the service configuration does not contain a sweep configuration
 */
extern acc_sweep_configuration_t acc_sweep_configuration_get(acc_service_configuration_t service_configuration);


/**
 * @brief Get the sensor id that is configured
 *
 * @param[in] configuration The sweep configuration to get the sensor id from
 * @return Sensor Id
 */
extern acc_sensor_id_t acc_sweep_configuration_sensor_get(acc_sweep_configuration_t configuration);


/**
 * @brief Set the sensor id
 *
 * @param[in] configuration The sweep configuration to set the sensor id in
 * @param[in] sensor_id The sensor id to set
 */
extern void acc_sweep_configuration_sensor_set(acc_sweep_configuration_t configuration, acc_sensor_id_t sensor_id);


/**
 * @brief Get the requested start of the range to sweep
 *
 * @param[in] configuration The sweep configuration to get the requested start from
 * @return Rquested start
 */
extern float acc_sweep_configuration_requested_start_get(acc_sweep_configuration_t configuration);


/**
 * @brief Set the requested start of the range to sweep
 *
 * @param[in] configuration The sweep configuration to set the requested start in
 * @param[in] start_m The requested start in meters
 */
extern void acc_sweep_configuration_requested_start_set(acc_sweep_configuration_t configuration, float start_m);


/**
 * @brief Get the requested length of the range to sweep
 *
 * @param[in] configuration The sweep configuration to get the requested length from
 * @return Requested length
 */
extern float acc_sweep_configuration_requested_length_get(acc_sweep_configuration_t configuration);


/**
 * @brief Set the requested length of the range to sweep
 *
 * @param[in] configuration The sweep configuration to set the requested length in
 * @param[in] length_m The requested length in meters
 */
extern void acc_sweep_configuration_requested_length_set(acc_sweep_configuration_t configuration, float length_m);


/**
 * @brief Set the requested range to sweep, both start and length
 *
 * @param[in] configuration The sweep configuration to set the requested range in
 * @param[in] start_m The requested start in meters
 * @param[in] length_m The requested length in meters
 */
extern void acc_sweep_configuration_requested_range_set(acc_sweep_configuration_t configuration, float start_m, float length_m);


/**
 * @brief Set the repetition mode for sweeps to streaming mode
 * 
 * In streaming mode the timing in the sensor hardware is used as the source for when
 * to perform sweeps.
 *
 * @param[in] configuration The sweep configuration to set streaming mode in
 * @param[in] sensor_sweep_frequency_hz The frequency to sweep with in hertz
 */
extern void acc_sweep_configuration_repetition_mode_streaming_set(acc_sweep_configuration_t configuration, float sensor_sweep_frequency_hz);


/**
 * @brief Get power save mode 
 *
 * @param[in] configuration The sweep configuration to get power save mode for
 * @return Power save mode
 */
extern acc_sweep_configuration_power_save_mode_t acc_sweep_configuration_power_save_mode_get(acc_sweep_configuration_t configuration);


/**
 * @brief Set sweep idle state
 * 
 * @param[in] configuration The sweep configuration to set power save mode in
 * @param[in] power_save_mode The power save mode to use
 */
extern void acc_sweep_configuration_power_save_mode_set(acc_sweep_configuration_t configuration, acc_sweep_configuration_power_save_mode_t power_save_mode);


#ifdef __cplusplus
}
#endif

#endif
