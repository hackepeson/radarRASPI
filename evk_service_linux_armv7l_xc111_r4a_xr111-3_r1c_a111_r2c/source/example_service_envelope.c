// Copyright (c) Acconeer AB, 2015-2018
// All rights reserved

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "acc_rss.h"
#include "acc_service.h"
#include "acc_service_envelope.h"
#include "acc_sweep_configuration.h"

#include "acc_os.h"
#include "acc_version.h"


static acc_service_status_t execute_envelope_with_blocking_calls(acc_service_configuration_t envelope_configuration);
static acc_service_status_t execute_envelope_with_callback(acc_service_configuration_t envelope_configuration);
static void envelope_callback(const acc_service_handle_t service_handle, const uint16_t *envelope_data, const acc_envelope_metadata_t *metadata, void *client_reference);
static void reconfigure_sweeps(acc_service_configuration_t envelope_configuration);


typedef struct
{
	struct acc_service_handle  *handle;
	uint16_t                   data_length;
	uint_fast8_t               sweep_count;
} envelope_callback_control_t;


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

	acc_service_status_t service_status;

	service_status = execute_envelope_with_blocking_calls(envelope_configuration);

	if (service_status != ACC_SERVICE_STATUS_OK) {
		printf("\nexecute_envelope_with_blocking_calls() => (%u) %s", (unsigned int)service_status, acc_service_status_name_get(service_status));
		return EXIT_FAILURE;
	}

	reconfigure_sweeps(envelope_configuration);

	service_status = execute_envelope_with_callback(envelope_configuration);

	if (service_status != ACC_SERVICE_STATUS_OK) {
		printf("\nexecute_envelope_with_callback() => (%u) %s", (unsigned int)service_status, acc_service_status_name_get(service_status));
		return EXIT_FAILURE;
	}

	acc_service_envelope_configuration_destroy(&envelope_configuration);

	acc_rss_deactivate();

	return EXIT_SUCCESS;
}


acc_service_status_t execute_envelope_with_blocking_calls(acc_service_configuration_t envelope_configuration)
{
	acc_service_handle_t handle = acc_service_create(envelope_configuration);

	if (handle == NULL) {
		printf("\nacc_service_create failed");
		return ACC_SERVICE_STATUS_FAILURE_UNSPECIFIED;
	}

	acc_service_envelope_metadata_t envelope_metadata;
	acc_service_envelope_get_metadata(handle, &envelope_metadata);

	printf("\nFree space absolute offset: %u mm", (unsigned int)(envelope_metadata.free_space_absolute_offset * 1000.0 + 0.5));
	printf("\nActual start: %u mm", (unsigned int)(envelope_metadata.actual_start_m * 1000.0 + 0.5));
	printf("\nActual length: %u mm", (unsigned int)(envelope_metadata.actual_length_m * 1000.0 + 0.5));
	printf("\nActual end: %u mm", (unsigned int)((envelope_metadata.actual_start_m + envelope_metadata.actual_length_m) * 1000.0 + 0.5));
	printf("\nData length: %u", (unsigned int)(envelope_metadata.data_length));

	uint16_t envelope_data[envelope_metadata.data_length];

	acc_service_status_t service_status = acc_service_activate(handle);

	if (service_status == ACC_SERVICE_STATUS_OK) {
		uint_fast8_t sweep_count = 5;

		while (sweep_count > 0) {
			service_status = acc_service_envelope_get_next(handle, envelope_data, envelope_metadata.data_length);

			if (service_status == ACC_SERVICE_STATUS_OK) {
				printf("\nEnvelope data:");
				for (uint_fast16_t index = 0; index < envelope_metadata.data_length; index++) {
					if (index && !(index % 8)) {
						printf("\n");
					}
					printf("%6u", (unsigned int)(envelope_data[index] + 0.5));
				}
				printf("\n");
			}
			else {
				printf("\nEnvelope data not properly retrieved");
			}

			sweep_count--;

			// Show what happens if application is late
			if (sweep_count == 1) {
				acc_os_sleep_us(200000);
			}
		}

		service_status = acc_service_deactivate(handle);
	}
	else {
		printf("\nacc_service_activate() %u => %s", (unsigned int)service_status, acc_service_status_name_get(service_status));
	}

	acc_service_destroy(&handle);

	return service_status;
}


acc_service_status_t execute_envelope_with_callback(acc_service_configuration_t envelope_configuration)
{
	envelope_callback_control_t callback_control;

	acc_service_envelope_envelope_callback_set(envelope_configuration, &envelope_callback, &callback_control);

	acc_service_handle_t handle = acc_service_create(envelope_configuration);

	if (handle == NULL) {
		printf("\nacc_service_create failed");
		return ACC_SERVICE_STATUS_FAILURE_UNSPECIFIED;
	}

	acc_service_envelope_metadata_t envelope_metadata;
	acc_service_envelope_get_metadata(handle, &envelope_metadata);

	printf("\nFree space absolute offset: %u mm", (unsigned int)(envelope_metadata.free_space_absolute_offset * 1000.0 + 0.5));
	printf("\nActual start: %u mm", (unsigned int)(envelope_metadata.actual_start_m * 1000.0 + 0.5));
	printf("\nActual length: %u mm", (unsigned int)(envelope_metadata.actual_length_m * 1000.0 + 0.5));
	printf("\nActual end: %u mm", (unsigned int)((envelope_metadata.actual_start_m + envelope_metadata.actual_length_m) * 1000.0 + 0.5));
	printf("\nData length: %u", (unsigned int)(envelope_metadata.data_length));

	// Configure callback control
	callback_control.handle = handle;
	callback_control.sweep_count = 2;
	callback_control.data_length = envelope_metadata.data_length;

	acc_service_status_t service_status = acc_service_activate(handle);

	if (service_status == ACC_SERVICE_STATUS_OK) {
		while (callback_control.sweep_count > 0) {
			acc_os_sleep_us(200);
		}

		service_status = acc_service_deactivate(handle);
	}
	else {
		printf("\nacc_service_activate() %u => %s", (unsigned int)service_status, acc_service_status_name_get(service_status));
	}

	acc_service_destroy(&handle);

	return service_status;
}


void envelope_callback(const acc_service_handle_t service_handle, const uint16_t *envelope_data, const acc_envelope_metadata_t *metadata, void *client_reference)
{
	envelope_callback_control_t *callback_control = client_reference;

	if (service_handle != callback_control->handle) {
		printf("\nenvelope_callback with invalid handle");
	}
	else {
		if (callback_control->sweep_count > 0) {
			printf("\nEnvelope callback metadata->sequence_number: %u", (unsigned int)metadata->sequence_number);
			printf("\nEnvelope callback data:");
			for (uint_fast16_t index = 0; index < callback_control->data_length; index++) {
				if (index && !(index % 8)) {
					printf("\n");
				}
				printf("%6u", (unsigned int)(envelope_data[index]));
			}
			printf("\n");

			callback_control->sweep_count--;
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
		float start_m = 0.4;
		float length_m = 0.5;
		float sweep_frequency_hz = 100;

		acc_sweep_configuration_requested_start_set(sweep_configuration, start_m);
		acc_sweep_configuration_requested_length_set(sweep_configuration, length_m);

		acc_sweep_configuration_repetition_mode_streaming_set(sweep_configuration, sweep_frequency_hz);
	}
}
