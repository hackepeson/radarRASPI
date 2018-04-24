// Copyright (c) Acconeer AB, 2018
// All rights reserved

#ifndef ACC_RSS_H_
#define ACC_RSS_H_

#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Activate the Acconeer Radar Services System, RSS
 *
 * The call to this function must only be made from one thread. After this, full thread safety across
 * all Acconeer services is guaranteed.
 *
 * @return bool True if RSS is activated
 */
extern bool acc_rss_activate(void);


/**
 * @brief Deactivate the Acconeer Radar Services System, RSS
 */
extern void acc_rss_deactivate(void);


/**
 * @brief Get the Acconeer RSS version
 * 
 * @return Version
 */
extern const char *acc_rss_version(void);


#ifdef __cplusplus
}
#endif

#endif
