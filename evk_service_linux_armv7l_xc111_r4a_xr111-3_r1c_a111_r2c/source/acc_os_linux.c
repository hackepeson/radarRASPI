// Copyright (c) Acconeer AB, 2016-2018
// All rights reserved

// needed for localtime_r
#define _POSIX_SOURCE

// needed for nanosleep
// needed for sigaction
// needed for siginfo_t
#define _POSIX_C_SOURCE 199309L

//needed for syscall
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include "acc_log.h"
#include "acc_os.h"


#define MODULE	"os"

#define ACC_OS_INVALID_SOCKET	(-1)

typedef struct acc_os_mutex {
	uint_fast8_t		is_initialized;
	pthread_mutex_t		mutex;
} acc_os_mutex_s;


typedef struct acc_os_semaphore {
	uint_fast8_t	  	is_initialized;
	sem_t			handle;
} acc_os_semaphore_s;


typedef struct acc_os_socket {
	int handle;
} acc_os_socket_s;


typedef struct acc_os_thread_handle {
	pthread_t handle;
}acc_os_thread_handle_s;


/**
 * @brief Flag set if stack has been prepared for usage measurement
 */
static uint_fast8_t acc_os_stack_setup_done = 0;


/**
 * @brief General signal handler registered by os_init()
 */
static void internal_signal_handler(int signum)
{
	if (signum == SIGINT)
		exit(0);
}


/**
 * @brief Perform any os specific initialization
 */
void acc_os_init(void)
{
	static bool init_done;

	if (init_done) {
		return;
	}

	// Logging with ACC_LOG is not possible within acc_os_init
	if (geteuid() == 0) {
		fprintf(stderr, "Executing with root privileges is not allowed!\n");
		exit(EACCES);
	}

	struct sigaction signal_action =
		{
		.sa_handler	= internal_signal_handler,
		.sa_flags	= 0
		};
	sigemptyset(&signal_action.sa_mask);
	if (sigaction(SIGINT, &signal_action, NULL) < 0) {
		fprintf(stderr, "Failed to setup signal handler for SIGINT, %s\n", strerror(errno));
	}

	init_done = true;
}


/**
 * @brief Prepare stack for measuring stack usage - to be called as early as possible
 *
 * @param stack_size Amount of stack in bytes that is allocated
 */
void acc_os_stack_setup(size_t stack_size)
{
	if (!stack_size)
		return;

	uint8_t stack_filler[stack_size];
	memset(stack_filler, 0x5a, sizeof(stack_filler));

	/* Prevent compiler from optimizing away stack_filler[] */
	__asm__ __volatile__("" :: "m" (stack_filler));

	acc_os_stack_setup_done = 1;
}


/**
 * @brief Measure amount of used stack in bytes
 *
 * @param stack_size Amount of stack in bytes that is allocated
 * @return Number of bytes of used stack space
 */
size_t acc_os_stack_get_usage(size_t stack_size)
{
	if (!stack_size || !acc_os_stack_setup_done)
		return 0;

	uint8_t stack_filler[stack_size];
	size_t	usage = 0;

	/* This does not give an exact figure but is useful as an indication of the used size.
	   The reason for counting backwards to allow future improvement handling any misalignment
	   garbage at the end. */
	for (size_t index = stack_size - 1; index; index--)
		if (stack_filler[index] != 0x5a)
			usage += 1;

	return usage;
}


/**
 * @brief Sleep for a specified number of microseconds
 *
 * @param time_usec Time in microseconds to sleep
 */
void acc_os_sleep_us(uint32_t time_usec)
{
	struct timespec	ts;

	if (time_usec == 0) {
		time_usec = 1;
	}

	ts.tv_sec  = time_usec / 1000000;
	ts.tv_nsec = (time_usec % 1000000) * 1000;

	nanosleep(&ts, NULL);
}


/**
 * @brief Allocate dynamic memory
 *
 * Use platform specific mechanism to allocate dynamic memory. The memory is guaranteed
 * to be naturally aligned. Requesting zero bytes will return NULL.
 *
 * On error, NULL is returned.
 *
 * @param size The number of bytes to allocate
 * @return Pointer to the allocated memory, or NULL if allocation failed
 */
void *acc_os_mem_alloc(size_t size)
{
	if (!size)
		return NULL;

	return malloc(size);
}


/**
 * @brief Free dynamic memory allocated with acc_os_mem_alloc
 *
 * Use platform specific mechanism to free dynamic memory. Passing NULL is allowed
 * but will do nothing.
 *
 * Freeing memory not allocated with acc_os_mem_alloc, or freeing memory already
 * freed, will result in undefined behaviour.
 *
 * @param ptr Pointer to the dynamic memory to free
 */
void acc_os_mem_free(void *ptr)
{
	free(ptr);
}


/**
 * @brief Return the unique thread ID for the current thread
 */
acc_os_thread_id_t acc_os_get_thread_id(void)
{
	return syscall(SYS_gettid);
}


/**
 * @brief Calculate current time and return in a struct tm, and optionally with microseconds
 *
 * @param time_tm	The local time, as struct tm, is returned here
 * @param time_usec	If not NULL, the microsecond part of the time is returned here
 */
void acc_os_localtime(struct tm *time_tm, uint32_t *time_usec)
{
	struct timeval	time_tv;

	int result = gettimeofday(&time_tv, NULL);
	if (result != 0) {
		ACC_LOG_ERROR("gettimeofday returned %d %d %s", result, errno, strerror(errno));
	}
	if (localtime_r(&time_tv.tv_sec, time_tm) == NULL) {
		ACC_LOG_ERROR("localtime_r return NULL");
	}
	if (time_usec)
		*time_usec = time_tv.tv_usec;
}


/**
 * @brief Create a mutex
 *
 * @return Newly initialized mutex
 */
acc_os_mutex_t acc_os_mutex_create(void)
{
	acc_os_mutex_t mutex = acc_os_mem_alloc(sizeof(*mutex));

	if (mutex != NULL) {
		pthread_mutex_init(&mutex->mutex, NULL);
		mutex->is_initialized = 1;
	}

	return mutex;
}


void acc_os_mutex_destroy(acc_os_mutex_t mutex)
{
	pthread_mutex_destroy(&mutex->mutex);
	acc_os_mem_free(mutex);
}


/**
 * @brief Mutex lock
 *
 * @param mutex Mutex to be locked
 */
void acc_os_mutex_lock(acc_os_mutex_t mutex)
{
	pthread_mutex_lock(&mutex->mutex);
}


/**
 * @brief Mutex unlock
 *
 * @param mutex Mutex to be unlocked
 */
void acc_os_mutex_unlock(acc_os_mutex_t mutex)
{
	pthread_mutex_unlock(&mutex->mutex);
}


/**
 * @brief Create new thread
 *
 * @param func	Function implementing the thread code
 * @param param	Parameter to be passed to the thread function
 * @return Newly created thread
 */
acc_os_thread_handle_t acc_os_thread_create(void (*func)(void *param), void *param)
{
	int	ret;

	acc_os_thread_handle_t thread = acc_os_mem_alloc(sizeof(*thread));

	if (thread != NULL) {
		// TODO the function cast should be replaced by something safer
		ret = pthread_create(&thread->handle, NULL, (void* (*)(void*))func, param);
		if (ret != 0) {
			ACC_LOG_ERROR("%s: Error %d, %s", __func__, ret, strerror(ret));
			return thread;
		}
	}

	ACC_LOG_VERBOSE("%s: created thread_handle=%lu", __func__, (unsigned long)thread->handle);
	return thread;
}


/**
 * @brief Delete current thread
 *
 * There is no return status. If the function returns, it failed.
 */
void acc_os_thread_delete(void)
{
	pthread_exit(NULL);
}


/**
 * @brief Cleanup after thread termination
 *
 * For operating systems that require it, perform any post-thread cleanup operation.
 *
 * @param thread Handle of thread
 */
void acc_os_thread_cleanup(acc_os_thread_handle_t thread)
{
	ACC_LOG_VERBOSE("acc_os_thread_cleanup: removed thread_handle=%lu", (unsigned long)thread->handle);
	// assume thread is already terminated, or just about to terminate
	pthread_join(thread->handle, NULL);
	acc_os_mem_free(thread);
}


/**
 * @brief Open a dynamic library, returning a handle to the library
 *
 * @param  filename Name of dynamic library file to be opened
 * @return A handle to be used for successive calls
 */
void *acc_os_dynamic_open(char *filename)
{
	return dlopen(filename, RTLD_LAZY);
}


/**
 * @brief Close a previously opened dynamic library
 *
 * @param  handle The handle returned from acc_os_dynamic_open()
 */
void acc_os_dynamic_close(void *handle)
{
	dlclose(handle);
}


/**
 * @brief Lookup a symbol in a dynamic library, returns NULL on failure
 *
 * @param  handle The handle returned from acc_os_dynamic_open()
 * @param  name   The name of the symbol to find in the library
 * @return The value of the symbol, if found
 */
void *acc_os_dynamic_symbol(void *handle, char *name)
{
	void	*addr;

	dlerror();	// discard any previous error
	addr = dlsym(handle, name);

	return addr;
}


/**
 * @brief Return a pointer to a string describing the last error, or NULL if no error
 *
 * @param  handle The handle returned by acc_os_dynamic_open()
 * @return A pointer to the error string
 */
char *acc_os_dynamic_error(void *handle __attribute__((unused)))
{
	return dlerror();
}


/**
 * @brief Translates an Internet hostname, such as www.acconeer.com to an IP-address.
 *
 * @param hostname The hostname to translate.
 * @param buf A buffer where the IP address will be saved.
 * @param buflen The len of buf.
 * @return 0 for success, -1 otherwise.
 */
static int acc_os_net_hostname_to_ip(const char *hostname , char* buf, size_t buflen)
{
	struct hostent *he;
	int res = -1;

	if (buflen < 16) {
		ACC_LOG_ERROR("%s Buffer too small, even for IPv4 addresses", __func__);
		return -1;
	}

	he = gethostbyname(hostname);
	if (he != NULL) {
		struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
		int i;

		for(i=0; addr_list[i]!=NULL; i++)
		{
			//Return the first;
			strncpy(buf, inet_ntoa(*addr_list[i]), buflen);
			buf[buflen-1] = '\0';

			res = 0;
			break;
		}
	}
	return res;
}


acc_os_net_address_t acc_os_net_string_to_address(const char *str)
{
	struct in_addr	addr;
	char ip[40]; // 39 is maxlen for IPv6
	const char *address = str; // "str" is default arg that will be sent to inet_pton
	int res;

	// Fetch IP if config argument is a hostname
	if (!acc_os_net_hostname_to_ip(str, ip, sizeof(ip))) {
		ACC_LOG_VERBOSE("Translated (%s) to (%s)", str, ip);
		address = ip; // Okay, we succeeded. Use "ip" as arg to inet_pton
	}

	res = inet_pton(AF_INET, address, &addr);
	if (res <= 0) {
		if (res == 0) {
			ACC_LOG_ERROR("inet_pton: Not in presentation format");
		} else {
			ACC_LOG_ERROR("Error from inet_pton %u (%u) %s", res, errno, strerror(errno));
		}
		return 0;
	}

	return addr.s_addr;
}


void acc_os_net_address_to_string(acc_os_net_address_t address, char *buffer, size_t max_size)
{
	struct in_addr	addr;
	char		name[INET_ADDRSTRLEN];

	if (!max_size)
		return;

	*buffer		= 0;
	addr.s_addr	= address;
	if (!inet_ntop(AF_INET, &addr, name, sizeof(name))) {
		ACC_LOG_ERROR("%s: inet_ntop(): (%u) %s", __func__, errno, strerror(errno));
		return;
	}

	snprintf(buffer, max_size, "%s", name);
	buffer[max_size - 1] = 0;
}


acc_os_socket_t acc_os_net_connect(acc_os_net_address_t address, acc_os_net_port_t port)
{
	acc_os_socket_t	sock = acc_os_mem_alloc(sizeof(*sock));

	if (sock == NULL) {
		return NULL;
	}

	int		sock_flags;

	if ((sock->handle = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		ACC_LOG_ERROR("%s: socket(AF_INET, SOCK_STREAM): (%u) %s", __func__, errno, strerror(errno));
		return NULL;
	}

	// make socket nonblocking to allow timeout handling for connect()
	sock_flags = fcntl(sock->handle, F_GETFL);
	fcntl(sock->handle, F_SETFL, sock_flags | O_NONBLOCK);

	// request connect, but don't block
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family		= AF_INET;
	addr.sin_addr.s_addr	= address;
	addr.sin_port		= htons(port);

	if ((connect(sock->handle, (struct sockaddr*)&addr, sizeof(addr)) < 0) && (errno != EINPROGRESS)) {
		char name[INET_ADDRSTRLEN];
		acc_os_net_address_to_string(address, name, sizeof(name));
		ACC_LOG_WARNING("%s: connect(%s:%u): (%u) %s", __func__, name, port, errno, strerror(errno));
		acc_os_net_disconnect(sock);
		return NULL;
	}

	// use select() to detect activity on socket, or timeout
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(sock->handle, &fdset);
	struct timeval connect_timeout = { .tv_sec = 0, .tv_usec = 500000 };
	int ret = select(sock->handle + 1, NULL, &fdset, NULL, &connect_timeout);
	if (ret < 0) {
		ACC_LOG_ERROR("%s: select() failed: (%u) %s", __func__, errno, strerror(errno));
		acc_os_net_disconnect(sock);
		return NULL;
	}
	if (!ret) {
		char name[INET_ADDRSTRLEN];
		acc_os_net_address_to_string(address, name, sizeof(name));
		ACC_LOG_WARNING("%s: connect() to %s:%u timed out", __func__, name, port);
		acc_os_net_disconnect(sock);
		return NULL;
	}

	// get any socket level errors
	int		so_error;
	socklen_t	len = sizeof(so_error);
	if (getsockopt(sock->handle, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
		ACC_LOG_ERROR("%s: getsockopt() failed: (%u) %s", __func__, errno, strerror(errno));
		acc_os_net_disconnect(sock);
		return NULL;
	}

	// verify that connect() succeeded
	if (so_error) {
		char name[INET_ADDRSTRLEN];
		acc_os_net_address_to_string(address, name, sizeof(name));
		ACC_LOG_ERROR("%s: so_error for %s:%u: (%u) %s", __func__, name, port, so_error, strerror(so_error));
		acc_os_net_disconnect(sock);
		return NULL;
	}

	// restore socket to blocking mode
	fcntl(sock->handle, F_SETFL, sock_flags & ~O_NONBLOCK);

	int value = 1;
	if (setsockopt(sock->handle, IPPROTO_TCP, TCP_NODELAY, (void*)&value, sizeof(value)) < 0) {
		ACC_LOG_WARNING("%s: message_driver_tcp: setsockopt(TCP_NODELAY): (%u) %s", __func__, errno, strerror(errno));
		acc_os_net_disconnect(sock);
		return NULL;
	}

	struct timeval send_timeout = { .tv_sec = 0, .tv_usec = 500000 };
	if (setsockopt(sock->handle, SOL_SOCKET, SO_SNDTIMEO, (void*)&send_timeout, sizeof(send_timeout)) < 0)
		ACC_LOG_WARNING("%s: setsockopt(SO_SNDTIMEO) failed: (%u) %s", __func__, errno, strerror(errno));

	return sock;
}


void acc_os_set_socket_invalid(acc_os_socket_t sock)
{
	if (sock != NULL) {
		sock->handle = ACC_OS_INVALID_SOCKET;
	}
}


bool acc_os_is_socket_valid(acc_os_socket_t sock)
{
	bool valid = false;

	if (sock != NULL) {
		valid = (sock->handle != ACC_OS_INVALID_SOCKET);
	}

	return valid;
}

void acc_os_net_disconnect(acc_os_socket_t sock)
{
	if (sock != NULL) {
		close(sock->handle);
	}
}


int acc_os_net_send(acc_os_socket_t sock, void *buffer, size_t size)
{
	ssize_t	result;
	ssize_t	remain = size;

	if (sock == NULL) {
		return -1;
	}

	while (remain) {
		while (((result = send(sock->handle, buffer, remain, MSG_NOSIGNAL)) < 0) && (errno == EINTR)) ;
		if (result < 0) {
			ACC_LOG_ERROR("%s(): (%u) %s", __func__, errno, strerror(errno));
			return -1;
		}

		if (!result) {
			ACC_LOG_ERROR("%s: send() sent zero bytes!", __func__);
			return -1;
		}

		buffer = (void*)((uint8_t*)buffer + result);
		remain -= result;
	}

	return size;
}


int acc_os_net_receive(acc_os_socket_t sock, void *buffer, size_t max_size, uint_fast32_t timeout_us)
{
	ssize_t	result;
	ssize_t	remain	= max_size;
	size_t	size	= 0;

	if (sock == NULL) {
		return -1;
	}

	struct timeval receive_timeout = { .tv_sec = timeout_us / 1000000UL, .tv_usec = timeout_us % 1000000UL };
	if (setsockopt(sock->handle, SOL_SOCKET, SO_RCVTIMEO, (void*)&receive_timeout, sizeof(receive_timeout)) < 0)
		ACC_LOG_WARNING("%s: setsockopt(SO_RCVTIMEO) failed: (%u) %s", __func__, errno, strerror(errno));

	while (remain) {
		while (((result = recv(sock->handle, buffer, remain, 0)) < 0) && (errno == EINTR)) ;

		if (result < 0) {
			ACC_LOG_ERROR("%s: (%u) %s", __func__, errno, strerror(errno));
			return -1;
		}

		// socket was closed by other end
		if (!result) {
			ACC_LOG_INFO("%s: Remote node closed connection", __func__);
			return -1;
		}

		buffer = (void*)((uint8_t*)buffer + result);
		remain	-= result;
		size	+= result;
	}

	return size;
}


acc_os_semaphore_t acc_os_semaphore_create(void)
{
	acc_os_semaphore_t sem = NULL;
	
	sem = acc_os_mem_alloc(sizeof(*sem));

	if (sem != NULL) {
		if (sem_init(&sem->handle, 0, 0) == -1) {
			return NULL;
		}
		sem->is_initialized	= 1;
	}

	return sem;
}


int_fast8_t acc_os_semaphore_wait(acc_os_semaphore_t sem, uint_fast16_t timeout_ms)
{
	struct timespec ts;
	struct timeval tv;

	if (sem == NULL || sem->is_initialized != 1) {
		ACC_LOG_ERROR("Not valid semaphore");
		return -1;
	}

	int result = gettimeofday(&tv, NULL);
	if (result != 0) {
		ACC_LOG_ERROR("gettimeofday returned %d %d %s", result, errno, strerror(errno));
		return -1;
	}

	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = tv.tv_usec + (timeout_ms * 1000);

	while (ts.tv_nsec >= 1000000) {
		ts.tv_sec++;
		ts.tv_nsec -= 1000000;
	}
	ts.tv_nsec *= 1000;

	if (sem_timedwait(&(sem->handle), &ts) == -1) {
		/* Timeout */
		if (errno == ETIMEDOUT) {
			if (timeout_ms != 0) {
				ACC_LOG_ERROR("Semaphore timeout");
			}
			return -1;
		}
		ACC_LOG_ERROR("An error has occured: %d", errno);
		return -1;
	}

	return 0;
}


void acc_os_semaphore_signal(acc_os_semaphore_t sem)
{
	if (sem != NULL && sem->is_initialized) {
		sem_post(&sem->handle);
	}
}


void acc_os_semaphore_signal_from_interrupt(acc_os_semaphore_t sem)
{
	acc_os_semaphore_signal(sem);
}


void acc_os_semaphore_destroy(acc_os_semaphore_t sem)
{
	if (sem != NULL && sem->is_initialized) {
		sem_destroy(&sem->handle);

		acc_os_mem_free(sem);
	}
}
