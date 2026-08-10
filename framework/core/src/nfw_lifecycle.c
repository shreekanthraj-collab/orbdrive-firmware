/**
 * @file nfw_lifecycle.c
 * @brief Framework initialization lifecycle implementation.
 */

#include "nfw_lifecycle.h"

/*===========================================================================*
 * Private Data
 *===========================================================================*/

static bool s_lifecycleInitialized = false;
static NfwLifecycleStage_t s_currentStage =
    NFW_LIFECYCLE_STAGE_RESET;

/*===========================================================================*
 * Public Functions
 *===========================================================================*/

NfwStatus_t nfwLifecycleInit(void)
{
    if (s_lifecycleInitialized)
    {
        return NFW_STATUS_ALREADY_INITIALIZED;
    }

    s_currentStage = NFW_LIFECYCLE_STAGE_RESET;
    s_lifecycleInitialized = true;

    return NFW_STATUS_OK;
}

NfwStatus_t nfwLifecycleAdvance(NfwLifecycleStage_t next_stage)
{
    if (!s_lifecycleInitialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    if (s_currentStage >= NFW_LIFECYCLE_STAGE_RUNNING)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    if (next_stage !=
        (NfwLifecycleStage_t)(s_currentStage + 1))
    {
        return NFW_STATUS_INVALID_STATE;
    }

    s_currentStage = next_stage;

    return NFW_STATUS_OK;
}

NfwLifecycleStage_t nfwLifecycleGetStage(void)
{
    return s_currentStage;
}

bool nfwLifecycleIsRunning(void)
{
    return s_currentStage == NFW_LIFECYCLE_STAGE_RUNNING;
}