// Copyright (c) Acconeer AB, 2016-2018
// All rights reserved

#ifndef ACC_OS_LINUX_H_
#define ACC_OS_LINUX_H_

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


#define ACC_OS_INVALID_SOCKET	(-1)


typedef struct {
	uint_fast8_t		is_initialized;
	pthread_mutex_t		mutex;
} acc_os_mutex_t;

typedef int		acc_os_socket_t;
typedef pthread_t	acc_os_thread_handle_t;

#ifdef __cplusplus
}
#endif

#endif
