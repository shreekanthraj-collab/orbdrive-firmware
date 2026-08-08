/**
 * @file nfw_time.h
 * @brief Platform time abstraction.
 *
 * This module provides a hardware-independent interface for obtaining
 * system time and performing delays.
 */

#ifndef NFW_TIME_H
#define NFW_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*===========================================================================
 * Public API
 *===========================================================================*/

/**
 * @brief Returns system uptime in milliseconds.
 *
 * @return Milliseconds since platform initialization.
 */
uint32_t nfwTimeNowMs(void);

/**
 * @brief Returns system uptime in microseconds.
 *
 * @return Microseconds since platform initialization.
 */
uint64_t nfwTimeNowUs(void);



#ifdef __cplusplus
}
#endif

#endif /* NFW_TIME_H */
