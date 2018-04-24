// Copyright (c) Acconeer AB, 2017-2018
// All rights reserved

#ifndef ACCONEER_DETECTOR_DISTANCE_H_
#define ACCONEER_DETECTOR_DISTANCE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "acc_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief The Acconeer distance detector provides generic detection of objects, i.e. no specific handling of size, shape
 * or distance to the object is taken into account.
 * 
 * The detector requires a threshold to detect against and a threshold may be provided using three different methods.
 * 1. Create an empty detector which require that threshold estimation is done before detection. This allows non-linear thresholds, but
 * also requires that any objects to detect are not present during threshold estimation. The estimated threshold is possible to retrieve
 * and save so that a detector may be recreated with the estimated threshold at a later time.
 * 2. Create the detector with a scalar value to be used as a fixed threshold.
 * 3. Create the detector with a provided threshold, requires that the provided threshold range is aligned with the detection range.
 * 
 * The detector provides two main functionalities, one is the ability to estimate a threshold and one is the detection towards a threshold.
 * Threshold estimation is required, and also only available, when an empty detector has been created and provides methods to update
 * the detector with multiple sweeps which will build up mean and standard deviation information for each data point in the sweep.
 * 
 * Detection is performed as a two-step procedure to allow application control of memory handling. The first step provides the
 * number of reflections detected and the second step provides the content of each of the reflections.
 */


/**
 * @brief Information on detected reflections is provided by this type
 * 
 * @param distance The distance to an object[m]
 * @param amplitude The amplitude of the reflection[arbitrary unit]
 */
typedef struct {
	float		distance;
	uint16_t	amplitude;
} acc_detector_distance_reflection_t;


/**
 * @brief Create an empty instance of a distance detector, i.e. that does not contain a threshold and where
 * threshold estimation is required.
 *
 * @return Detector reference, NULL if creation fails
 */
extern void *acc_detector_distance_create_empty(void);


/**
 * @brief Create an instance of a distance detector with a fix threshold defined by a value.
 *
 * @param[in] fix_threshold_value The fix threshold value to use as a threshold for all data points
 * 
 * @return Detector reference, NULL if creation fails
 */
extern void *acc_detector_distance_create_fixed(uint16_t fix_threshold_value);


/**
 * @brief Create an instance of a distance detector with a provided threshold.
 *
 * @param[in] threshold_context_size The size of the threshold_context_data
 * @param[in] threshold_context_data A reference to a context from a previous threshold estimation
 * 
 * @return Detector reference, NULL if creation fails
 */
extern void *acc_detector_distance_create_with_threshold(size_t threshold_context_size, void *threshold_context_data);


/**
 * @brief Destroy an instance of a distance detector.
 * 
 * @param[in] detector A reference to the detector to destroy
 */
extern void acc_detector_distance_destroy(void *detector);


/**
 * @brief Set sensitivity factor for false detection rate, 0.0 for lowest sensitivity and 1.0 for highest sensitivity
 *
 * This function is optional but, if used, must be called before acc_detector_distance_detect().
 *
 * @param[in] detector A reference to the detector whose sensitivity parameter will be set
 * @param[in] sensitivity Sensitivity factor to be set
 * @return Status
 */
extern acc_status_t acc_detector_distance_set_sensitivity(void *detector, float sensitivity);


/**
 * @brief Set if detector should operate on absolute amplitude (otherwise delta amplitude compared to threshold, which is the default behaviour)
 *
 * This function is optional but, if used, must be called before acc_detector_distance_detect().
 *
 * @param[in] detector A reference to the detector where relative or absolute amplitude should be used
 * @param[in] set_absolute True if absolute amplitude should be used instead of difference (compared to threshold), false otherwise
 * @return Status
 */
extern acc_status_t acc_detector_distance_set_absolute_amplitude(void *detector, bool set_absolute);


/**
 * @brief Threshold estimation update.
 *
 * Starts or updates an already ongoing threshold estimation.
 * It is recommended to use at least 50 updates.
 *
 * @param[in] detector A reference to the detector
 * @param[in] distance_start_m The distance of the first data point[m]
 * @param[in] distance_end_m The distance of the last data point[m]
 * @param[in] data_size The size of data
 * @param[in] data The data to use for threshold estimation
 * 
 * @return Status
 */
extern acc_status_t acc_detector_distance_threshold_estimation_update(void *detector, float distance_start_m, float distance_end_m, uint16_t data_size, uint16_t *data);


/**
 * @brief Threshold estimation reset.
 *
 * Resets contents of any ongoing threshold estimation.
 *
 * @param[in] detector A reference to the detector
 * 
 * @return Status
 */
extern acc_status_t acc_detector_distance_threshold_estimation_reset(void *detector);


/**
 * @brief Get the size of threshold estimation data.
 *
 * Retrieves the size of threshold estimation data to use when retrieving the data.
 *
 * @param[in] detector A reference to the detector
 * @param[out] threshold_data_size The threshold estimation data size is stored in this parameter
 * 
 * @return Status
 */
extern acc_status_t acc_detector_distance_threshold_estimation_get_size(void *detector, size_t *threshold_data_size);


/**
 * @brief Get the result of a threshold estimation.
 *
 * Retrieves the result of a threshold estimation which is possible to save to a file and use when recreating the detector.
 *
 * @param[in] detector A reference to the detector
 * @param[in] threshold_data_size The size of memory referenced by threshold_data, must be of at least the size provided in acc_detector_distance_threshold_estimation_get_size
 * @param[out] threshold_data The threshold data is stored in this parameter, must be of at least the size provided in acc_detector_distance_threshold_estimation_get_size
 * 
 * @return Status
 */
extern acc_status_t acc_detector_distance_threshold_estimation_get_data(void *detector, size_t threshold_data_size, uint8_t *threshold_data);


/**
 * @brief Detect reflections in the provided data.
 *
 * Uses the threshold provided by threshold estimation or, if no threshold estimation has been performed,
 * the default threshold value for detecting reflections. Provides how many reflections that has been detected, calling
 * acc_detector_distance_get_reflections provides the content of each refleciton for last detection.
 *
 * Note that this function may alter the input data.
 * 
 * @param[in] detector A reference to the detector
 * @param[in] distance_start_m The distance of the first data point[m]
 * @param[in] distance_end_m The distance of the last data point[m]
 * @param[in] data_size The size of data
 * @param[in] data The data to perform detection on, may be altered
 * @param[out] reflection_count The number of detected reflections are provided in this parameter
 *
 * @return Status
 */
extern acc_status_t acc_detector_distance_detect(void *detector, float distance_start_m, float distance_end_m, uint16_t data_size, uint16_t *data, uint16_t *reflection_count);


/**
 * @brief Get reflections from a previous acc_detector_distance_detect call.
 *
 * The provided array shall be at least the returned reflection_count from acc_detector_distance_detect in length.
 * 
 * @param[in] detector A reference to the detector
 * @param[in] reflections_length The length of the reflections array, must be at least reflection_count provided by acc_detector_distance_detect in length
 * @param[out] reflections The content of each detected reflection is stored in this parameter, must be at least reflection_count provided by acc_detector_distance_detect in length
 *
 * @return Status
 */
extern acc_status_t acc_detector_distance_get_reflections(void *detector, uint16_t reflections_length, acc_detector_distance_reflection_t *reflections);

#ifdef __cplusplus
}
#endif

#endif
