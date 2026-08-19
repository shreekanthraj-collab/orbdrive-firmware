#ifndef VOLTAGE_CONFIG_H
#define VOLTAGE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "nfw_status.h"

typedef struct
{
    float warningLowV;
    float warningHighV;
    float cutoffBypassV;
    float criticalV;
    float resetV;

    uint32_t bypassTimeoutMs;

} VoltageConfig_t;

/*
 * Firmware fallback defaults.
 *
 * These are used only when no valid EOL/NVS configuration exists.
 */
#define VOLTAGE_CONFIG_DEFAULT_WARNING_LOW_V       (12.0f)
#define VOLTAGE_CONFIG_DEFAULT_WARNING_HIGH_V      (12.4f)
#define VOLTAGE_CONFIG_DEFAULT_CUTOFF_BYPASS_V     (11.6f)
#define VOLTAGE_CONFIG_DEFAULT_CRITICAL_V          (11.2f)
#define VOLTAGE_CONFIG_DEFAULT_RESET_V             (12.0f)
#define VOLTAGE_CONFIG_DEFAULT_BYPASS_TIMEOUT_MS   (120000UL)

/*
 * Absolute firmware safety boundaries.
 *
 * EOL and Gateway configuration must remain within these
 * firmware-enforced limits.
 */
#define VOLTAGE_CONFIG_ABSOLUTE_MIN_V              (10.0f)
#define VOLTAGE_CONFIG_ABSOLUTE_MAX_V              (15.0f)

#define VOLTAGE_CONFIG_ABSOLUTE_MIN_TIMEOUT_MS      (1000UL)
#define VOLTAGE_CONFIG_ABSOLUTE_MAX_TIMEOUT_MS      (600000UL)

/**
 * @brief Initialize voltage configuration.
 *
 * Loads the active EOL/Gateway configuration from NVS.
 * If no valid configuration exists, firmware fallback values
 * are used.
 */
NfwStatus_t voltageConfigInit(void);

/**
 * @brief Load the active voltage configuration.
 *
 * @param config Pointer receiving the active configuration.
 */
NfwStatus_t voltageConfigLoad(
    VoltageConfig_t *config);

/**
 * @brief Get the active voltage configuration.
 *
 * @param config Pointer receiving the active configuration.
 */
NfwStatus_t voltageConfigGet(
    VoltageConfig_t *config);

/**
 * @brief Save a validated voltage configuration to NVS.
 *
 * This API is used by both EOL factory provisioning and
 * later Gateway configuration.
 *
 * @param config Configuration to persist.
 */
NfwStatus_t voltageConfigSave(
    const VoltageConfig_t *config);

/**
 * @brief Validate voltage configuration against firmware
 *        safety boundaries and internal threshold ordering.
 *
 * @param config Configuration to validate.
 */
NfwStatus_t voltageConfigValidate(
    const VoltageConfig_t *config);

#ifdef __cplusplus
}
#endif

#endif /* VOLTAGE_CONFIG_H */