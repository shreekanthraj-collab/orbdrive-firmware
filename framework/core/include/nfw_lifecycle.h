/**
 * @file nfw_lifecycle.h
 * @brief Framework initialization lifecycle.
 */

#ifndef NFW_LIFECYCLE_H
#define NFW_LIFECYCLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "nfw_status.h"

typedef enum
{
    NFW_LIFECYCLE_STAGE_RESET = 0,

    NFW_LIFECYCLE_STAGE_HAL_INIT,

    NFW_LIFECYCLE_STAGE_PLATFORM_INIT,

    NFW_LIFECYCLE_STAGE_SERVICES_INIT,

    NFW_LIFECYCLE_STAGE_APPLICATION_INIT,

    NFW_LIFECYCLE_STAGE_RUNNING

} NfwLifecycleStage_t;

/**
 * @brief Initializes the lifecycle manager.
 *
 * The initial lifecycle stage is RESET.
 *
 * @return NFW_STATUS_OK on first initialization.
 * @return NFW_STATUS_ALREADY_INITIALIZED if already initialized.
 */
NfwStatus_t nfwLifecycleInit(void);

/**
 * @brief Advances the lifecycle to the next valid stage.
 *
 * Only the immediately following lifecycle stage is accepted.
 *
 * @param next_stage Requested next lifecycle stage.
 *
 * @return NFW_STATUS_OK if the transition is valid.
 * @return NFW_STATUS_INVALID_STATE if the transition is invalid.
 */
NfwStatus_t nfwLifecycleAdvance(
    NfwLifecycleStage_t next_stage
);

/**
 * @brief Returns the current lifecycle stage.
 *
 * @return Current lifecycle stage.
 */
NfwLifecycleStage_t nfwLifecycleGetStage(void);

/**
 * @brief Returns whether the lifecycle has reached RUNNING.
 *
 * @return true if the current stage is RUNNING.
 * @return false otherwise.
 */
bool nfwLifecycleIsRunning(void);

#ifdef __cplusplus
}
#endif

#endif /* NFW_LIFECYCLE_H */