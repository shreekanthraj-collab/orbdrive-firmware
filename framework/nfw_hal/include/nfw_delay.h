/**
 * @file nfw_delay.h
 * @brief Orb Drive delay abstraction.
 */

#ifndef NFW_DELAY_H
#define NFW_DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "nfw_status.h"

/**
 * @brief Delay execution for a number of milliseconds.
 *
 * @param milliseconds Delay duration.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwDelayMs(
    uint32_t milliseconds);

/**
 * @brief Delay execution for a number of microseconds.
 *
 * @param microseconds Delay duration.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwDelayUs(
    uint32_t microseconds);

#ifdef __cplusplus
}
#endif

#endif /* NFW_DELAY_H */