// Copyright (c) Acconeer AB, 2016-2018
// All rights reserved

#ifndef ACC_LOG_H_
#define ACC_LOG_H_

#include "acc_types.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	ACC_LOG_LEVEL_FATAL,
	ACC_LOG_LEVEL_ERROR,
	ACC_LOG_LEVEL_WARNING,
	ACC_LOG_LEVEL_INFO,
	ACC_LOG_LEVEL_VERBOSE,
	ACC_LOG_LEVEL_DEBUG,
	ACC_LOG_LEVEL_MAX
} acc_log_level_enum_t;
typedef uint32_t acc_log_level_t;


#define ACC_LOG(level, ...)	acc_log(level, MODULE, __VA_ARGS__)

#define ACC_LOG_FATAL(...)	ACC_LOG(ACC_LOG_LEVEL_FATAL, __VA_ARGS__)
#define ACC_LOG_ERROR(...)	ACC_LOG(ACC_LOG_LEVEL_ERROR, __VA_ARGS__)
#define ACC_LOG_WARNING(...)	ACC_LOG(ACC_LOG_LEVEL_WARNING, __VA_ARGS__)
#define ACC_LOG_INFO(...)	ACC_LOG(ACC_LOG_LEVEL_INFO, __VA_ARGS__)
#define ACC_LOG_VERBOSE(...)	ACC_LOG(ACC_LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define ACC_LOG_DEBUG(...)	ACC_LOG(ACC_LOG_LEVEL_DEBUG, __VA_ARGS__)


/**
 * @brief Return a string describing a status code
 */
extern char *acc_log_status_name(acc_status_t status);

extern void acc_log_set_level(acc_log_level_t level, char *module);
extern void acc_log(acc_log_level_t level, char *module, char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
