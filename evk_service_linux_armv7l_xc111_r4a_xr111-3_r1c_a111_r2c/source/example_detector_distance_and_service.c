// Copyright (c) Acconeer AB, 2018
// All rights reserved

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "acconeer_detector_distance.h"
#include "acc_rss.h"
#include "acc_service.h"
#include "acc_service_envelope.h"
#include "acc_sweep_configuration.h"
#include "acc_types.h"
#include "acc_version.h"


#define FIXED_THRESHOLD_VALUE   (1500)
#define SENSOR_ID               (1)
#define RANGE_START_M           (0.2)
#define RANGE_LENGTH_M          (0.5)


static void measure_distance(acc_service_handle_t envelope_handle, acc_service_envelope_metadata_t *envelope_metadata, void *distance_detector);
static void reconfigure_sweeps(acc_service_configuration_t envelope_configuration);
static char *format_distances(uint16_t reflection_count, const acc_detector_distance_reflection_t *reflections, float sensor_offset);


int main(int argc, char *argv[])
{
	ACC_UNUSED(argc);
	ACC_UNUSED(argv);

	printf("\nAcconeer software version %s", ACC_VERSION);
	printf("\nAcconeer RSS version %s", acc_rss_version());

	if (!acc_rss_activate()) {
		return EXIT_FAILURE;
	}

	acc_service_configuration_t envelope_configuration = acc_service_envelope_configuration_create();

	if (envelope_configuration == NULL) {
		printf("\nacc_service_envelope_configuration_create() failed");
		return EXIT_FAILURE;
	}

	reconfigure_sweeps(envelope_configuration);

	acc_service_handle_t envelope_handle = acc_service_create(envelope_configuration);

	if (envelope_handle == NULL) {
		printf("\nacc_service_create failed");
		return EXIT_FAILURE;
	}

	acc_service_envelope_metadata_t envelope_metadata;
	acc_service_envelope_get_metadata(envelope_handle, &envelope_metadata);

	printf("\nFree space absolute offset: %u mm", (unsigned int)(envelope_metadata.free_space_absolute_offset * 1000.0 + 0.5));
	printf("\nActual start: %u mm", (unsigned int)(envelope_metadata.actual_start_m * 1000.0 + 0.5));
	printf("\nActual length: %u mm", (unsigned int)(envelope_metadata.actual_length_m * 1000.0 + 0.5));
	printf("\nActual end: %u mm", (unsigned int)((envelope_metadata.actual_start_m + envelope_metadata.actual_length_m) * 1000.0 + 0.5));
	printf("\nData length: %u", (unsigned int)(envelope_metadata.data_length));
	printf("\nSensor: %u", (unsigned int)(SENSOR_ID));

	void *distance_detector = acc_detector_distance_create_fixed(FIXED_THRESHOLD_VALUE);

	if (distance_detector == NULL) {
		printf("\nCould not create distance_detector");
		return EXIT_FAILURE;
	}

	acc_service_status_t service_status;

	service_status = acc_service_activate(envelope_handle);

	if (service_status == ACC_SERVICE_STATUS_OK) {
		for (;;) {
			measure_distance(envelope_handle, &envelope_metadata, distance_detector);
		}
	}
	else {
		printf("\nacc_service_activate() %u => %s", (unsigned int)service_status, acc_service_status_name_get(service_status));
	}

	service_status = acc_service_deactivate(envelope_handle);

	acc_service_destroy(&envelope_handle);

	acc_detector_distance_destroy(distance_detector);

	acc_service_envelope_configuration_destroy(&envelope_configuration);

	acc_rss_deactivate();

	return EXIT_SUCCESS;
}


void measure_distance(acc_service_handle_t envelope_handle, acc_service_envelope_metadata_t *envelope_metadata, void *distance_detector)
{
	float start_m = envelope_metadata->actual_start_m;
	float end_m = envelope_metadata->actual_start_m + envelope_metadata->actual_length_m;

	uint16_t envelope_data[envelope_metadata->data_length];

	acc_service_status_t service_status = acc_service_envelope_get_next(envelope_handle, envelope_data, envelope_metadata->data_length);

	if (service_status != ACC_SERVICE_STATUS_OK) {
		printf("\nacc_service_envelope_get_next() => (%u) %s", (unsigned int)service_status, acc_service_status_name_get(service_status));
	}
	else {
		acc_status_t   status;
		uint16_t       reflection_count;

		status = acc_detector_distance_detect(distance_detector, start_m, end_m, envelope_metadata->data_length, envelope_data, &reflection_count);

		if (status != ACC_STATUS_SUCCESS) {
			printf("\nacc_detector_distance_detect() => (%u)", (unsigned int)status);
		}
		else if (reflection_count == 0) {
			printf("\nDetector distance (%u-%u mm): No object found",
				(unsigned int)(start_m * 1000.0 + 0.5),
				(unsigned int)(end_m * 1000.0 + 0.5));
		}
		else {
			acc_detector_distance_reflection_t reflections[reflection_count];

			status = acc_detector_distance_get_reflections(distance_detector, reflection_count, reflections);

			if (status != ACC_STATUS_SUCCESS) {
				printf("\nacc_detector_distance_get_reflections() => (%u)", (unsigned int)status);
			}
			else {
				printf("\nDetector distance (%u-%u mm): %s",
					(unsigned int)(start_m * 1000.0 + 0.5),
					(unsigned int)(end_m * 1000.0 + 0.5),
					format_distances(reflection_count, reflections, envelope_metadata->free_space_absolute_offset));
			}
		}

	}
}


void reconfigure_sweeps(acc_service_configuration_t envelope_configuration)
{
	acc_sweep_configuration_t sweep_configuration = acc_sweep_configuration_get(envelope_configuration);

	if (sweep_configuration == NULL) {
		printf("\nSweep configuration not available");
	}
	else {
		acc_service_envelope_profile_set(envelope_configuration, ACC_SERVICE_ENVELOPE_PROFILE_LONG_RANGE);

		acc_sweep_configuration_sensor_set(sweep_configuration, SENSOR_ID);
		acc_sweep_configuration_requested_start_set(sweep_configuration, RANGE_START_M);
		acc_sweep_configuration_requested_length_set(sweep_configuration, RANGE_LENGTH_M);
	}
}


char *format_distances(uint16_t reflection_count, const acc_detector_distance_reflection_t *reflections, float sensor_offset)
{
	static char	buffer[1024];
	size_t		total_count = 0;
	int		count;

	*buffer = 0;
	for (uint_fast8_t reflection_index = 0; reflection_index < reflection_count; reflection_index++) {
		if (total_count > 0) {
			count = snprintf(&buffer[total_count], sizeof(buffer) - total_count, ", ");
			if (count < 0) {
				break;
			}
			total_count += count;
		}

		count = snprintf(&buffer[total_count], sizeof(buffer) - total_count, "%u mm (%u)",
				(unsigned int)((reflections[reflection_index].distance - sensor_offset) * 1000.0 + 0.5),
				(unsigned int)(reflections[reflection_index].amplitude + 0.5));
		if (count < 0) {
			break;
		}
		total_count += count;

		if (total_count >= sizeof(buffer) - 1) {
			break;
		}
	}

	return buffer;
}
