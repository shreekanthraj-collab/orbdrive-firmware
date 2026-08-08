/**
 * @file nfw_mutex.h
 * @brief Platform mutex abstraction.
 */

#ifndef NFW_MUTEX_H
#define NFW_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "nfw_types.h"

/*===========================================================================
 * Types
 *===========================================================================*/

/**
 * @brief Opaque mutex handle.
 */
typedef NfwHandle_t NfwMutex_t;

/*===========================================================================
 * Public API
 *===========================================================================*/

/**
 * @brief Creates a mutex.
 *
 * @return Mutex handle or NULL on failure.
 */
NfwMutex_t nfwMutexCreate(void);

/**
 * @brief Deletes a mutex.
 *
 * @param mutex Mutex handle.
 */
void nfwMutexDelete(NfwMutex_t mutex);

/**
 * @brief Locks a mutex.
 *
 * @param mutex Mutex handle.
 *
 * @return true if successful.
 */
bool nfwMutexLock(NfwMutex_t mutex);

/**
 * @brief Unlocks a mutex.
 *
 * @param mutex Mutex handle.
 */
void nfwMutexUnlock(NfwMutex_t mutex);

#ifdef __cplusplus
}
#endif

#endif /* NFW_MUTEX_H */
