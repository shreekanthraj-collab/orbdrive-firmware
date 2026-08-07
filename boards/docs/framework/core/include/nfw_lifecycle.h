/**
 * @file nfw_lifecycle.h
 * @brief Framework initialization lifecycle.
 */

#ifndef NFW_LIFECYCLE_H
#define NFW_LIFECYCLE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    NFW_LIFECYCLE_STAGE_RESET = 0,

    NFW_LIFECYCLE_STAGE_HAL_INIT,

    NFW_LIFECYCLE_STAGE_PLATFORM_INIT,

    NFW_LIFECYCLE_STAGE_SERVICES_INIT,

    NFW_LIFECYCLE_STAGE_APPLICATION_INIT,

    NFW_LIFECYCLE_STAGE_RUNNING

} NfwLifecycleStage_t;

#ifdef __cplusplus
}
#endif

#endif /* NFW_LIFECYCLE_H */