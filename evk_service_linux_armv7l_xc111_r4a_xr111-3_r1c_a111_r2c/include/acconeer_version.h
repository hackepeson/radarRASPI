// Copyright (c) Acconeer AB, 2017-2018
// All rights reserved

#ifndef ACCONEER_VERSION_H_
#define ACCONEER_VERSION_H_

#include "acc_types_version.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Request to get Acconeer software version information
 *
 * @param version The version is returned in this parameter
*/
extern void acc_version_get(acc_version_t *version);

#ifdef __cplusplus
}
#endif

#endif
