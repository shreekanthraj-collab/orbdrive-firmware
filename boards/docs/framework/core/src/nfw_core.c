/**
 * @file nfw_core.c
 * @brief Orb Drive Framework Core implementation.
 */

#include "nfw_core.h"

static bool s_initialized = false;

NfwStatus_t nfwCoreInit(void)
{
    if (s_initialized)
    {
        return NFW_STATUS_ALREADY_INITIALIZED;
    }

    s_initialized = true;

    return NFW_STATUS_OK;
}

bool nfwCoreIsInitialized(void)
{
    return s_initialized;
}
