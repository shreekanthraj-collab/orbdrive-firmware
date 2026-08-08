/**
 * @file nfw_platform.c
 * @brief Orb Drive Platform implementation.
 */

#include "nfw_platform.h"

#include "nfw_core_internal.h"

/*===========================================================================
 * Private Data
 *===========================================================================*/

static bool s_platformInitialized = false;

/*===========================================================================
 * Public Functions
 *===========================================================================*/

NfwStatus_t nfwPlatformInit(void)
{
    if (s_platformInitialized)
    {
        return NFW_STATUS_OK;
    }

    /*
     * Future initialization:
     *  - Time subsystem
     *  - Memory subsystem
     *  - Mutex subsystem
     *  - Platform-specific startup
     */

    s_platformInitialized = true;

    return NFW_STATUS_OK;
}

bool nfwPlatformIsInitialized(void)
{
    return s_platformInitialized;
}
