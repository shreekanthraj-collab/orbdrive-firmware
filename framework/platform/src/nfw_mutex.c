/**
 * @file nfw_mutex.c
 * @brief Platform mutex abstraction implementation.
 */

#include "nfw_mutex.h"

/*===========================================================================
 * Public Functions
 *===========================================================================*/

NfwMutex_t nfwMutexCreate(void)
{
    return NULL;
}

void nfwMutexDelete(NfwMutex_t mutex)
{
    (void)mutex;
}

bool nfwMutexLock(NfwMutex_t mutex)
{
    (void)mutex;
    return true;
}

void nfwMutexUnlock(NfwMutex_t mutex)
{
    (void)mutex;
}
