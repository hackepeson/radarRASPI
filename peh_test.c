// Copyright (c) Acconeer AB, 2015-2018
// All rights reserved

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "acc_rss.h"
#include "acc_service.h"
#include "acc_service_envelope.h"
#include "acc_sweep_configuration.h"

#include "acc_os.h"
#include "acc_version.h"

void reconfigure_sweeps(acc_service_configuration_t envelope_configuration)
static acc_service_status_t execute_envelope_with_blocking_calls(acc_service_configuration_t envelope_configuration);

// UDP
#define SERVER "192.168.0.107"
#define BUFLEN 1024  //Max length of buffer
#define PORT 8888   //The port on which to send data
struct sockaddr_in si_other;
int s;
int i;
int slen=sizeof(si_other);
uint16_t message[BUFLEN];

void die(char *s)
{
    perror(s);
    exit(1);
}


int main(int argc, char *argv[])
{
  for (int n = 0; n < BUFLEN; n++)
  {
    message[n] = n;
  }
  
  // UDP Stuff	
  if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
  {
    die("socket");
  }
 
  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;
  si_other.sin_port = htons(PORT);
     
  if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
  {
    fprintf(stderr, "inet_aton() failed\n");
    exit(1);
  }

  ACC_UNUSED(argc);
  ACC_UNUSED(argv);

  printf("\nAcconeer software version %s", ACC_VERSION);
  printf("\nAcconeer RSS version %s", acc_rss_version());

  if (!acc_rss_activate()) 
  {
    return EXIT_FAILURE;
  }

  acc_service_configuration_t envelope_configuration = acc_service_envelope_configuration_create();

  if (envelope_configuration == NULL)
  {
    printf("\nacc_service_envelope_configuration_create() failed");
    return EXIT_FAILURE;
  }

  acc_service_status_t service_status;
  reconfigure_sweeps(envelope_configuration);

  service_status = execute_envelope_with_blocking_calls(envelope_configuration);
  

  if (service_status != ACC_SERVICE_STATUS_OK) 
  {
    printf("\nexecute_envelope_with_blocking_calls() => (%u) %s", (unsigned int)service_status, acc_service_status_name_get(service_status));
    return EXIT_FAILURE;
  }

 
  acc_service_envelope_configuration_destroy(&envelope_configuration);

  acc_rss_deactivate();

  return EXIT_SUCCESS;
}


acc_service_status_t execute_envelope_with_blocking_calls(acc_service_configuration_t envelope_configuration)
{
  acc_service_handle_t handle = acc_service_create(envelope_configuration);

  if (handle == NULL) 
  {
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

  if (service_status == ACC_SERVICE_STATUS_OK)
  {
    while (1) 
    {
      service_status = acc_service_envelope_get_next(handle, envelope_data, envelope_metadata.data_length);
      usleep(100);

      if (service_status == ACC_SERVICE_STATUS_OK) 
      {
	printf("\nEnvelope data:");
	for (uint_fast16_t index = 0; index < envelope_metadata.data_length; index++) 
        {
	  printf("%d %6u\n", index,(unsigned int)(envelope_data[index] + 0.5));
          message[index] = (int16_t)envelope_data[index] + 0.5;
	}
	printf("\n");
        if (sendto(s, message, sizeof(message) , 0 , (struct sockaddr *) &si_other, slen)==-1)
        {
          die("sendto()");
        }
      }
      else
      {
	printf("\nEnvelope data not properly retrieved");
      }
    }
    service_status = acc_service_deactivate(handle);
  }
  else 
  {
    printf("\nacc_service_activate() %u => %s", (unsigned int)service_status, acc_service_status_name_get(service_status));
  }

  acc_service_destroy(&handle);
  close(s);

  return service_status;
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

