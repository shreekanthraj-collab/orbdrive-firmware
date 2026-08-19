#ifndef OC_CONFIG_H
#define OC_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nfw_status.h"

typedef struct
{
    float minimumSafeCurrentA;
    float maximumSafeCurrentA;
} OcCurrentConfig_t;

/*
 * Firmware fallback values.
 *
 * These are used only when no valid EOL/NVS configuration exists.
 */
#define OC_CONFIG_DEFAULT_MIN_CURRENT_A    (1.0f)
#define OC_CONFIG_DEFAULT_MAX_CURRENT_A    (5.0f)

/*
 * Absolute firmware safety boundaries.
 *
 * EOL and Gateway configuration must remain within these limits.
 */
#define OC_CONFIG_ABSOLUTE_MIN_CURRENT_A  (0.1f)
#define OC_CONFIG_ABSOLUTE_MAX_CURRENT_A  (8.0f)

/**
 * @brief Initialize OC configuration.
 *
 * Loads the production configuration from NVS.
 * If no valid configuration exists, firmware fallback values are used.
 */
NfwStatus_t ocConfigInit(void);

/**
 * @brief Load the active OC configuration.
 *
 * @param config Pointer receiving the active configuration.
 */
NfwStatus_t ocConfigLoad(
    OcCurrentConfig_t *config);

/**
 * @brief Save a validated OC configuration to NVS.
 *
 * This is the API that EOL provisioning and later
 * Gateway configuration will use.
 *
 * @param config Configuration to persist.
 */
NfwStatus_t ocConfigSave(
    const OcCurrentConfig_t *config);

/**
 * @brief Get the currently active OC configuration.
 *
 * @param config Pointer receiving the active configuration.
 */
NfwStatus_t ocConfigGet(
    OcCurrentConfig_t *config);

/**
 * @brief Validate an OC configuration against
 *        firmware safety boundaries.
 *
 * @param config Configuration to validate.
 */
NfwStatus_t ocConfigValidate(
    const OcCurrentConfig_t *config);

#ifdef __cplusplus
}
#endif

#endif /* OC_CONFIG_H */