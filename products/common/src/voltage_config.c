/**
 * @file voltage_config.c
 * @brief Persistent EOL/Gateway-configurable voltage protection limits.
 */

#include "voltage_config.h"

#include <stdbool.h>
#include <stddef.h>

#include "nvs.h"
#include "nvs_flash.h"

/* ============================================================================
 * NVS configuration
 * ========================================================================== */

#define VOLTAGE_CONFIG_NAMESPACE       "orb_voltage"
#define VOLTAGE_CONFIG_FORMAT_VERSION  (1U)

#define KEY_FORMAT_VERSION             "fmt_ver"
#define KEY_WARNING_LOW                "warn_low"
#define KEY_WARNING_HIGH               "warn_high"
#define KEY_CUTOFF_BYPASS              "cut_bypass"
#define KEY_CRITICAL                   "critical"
#define KEY_RESET                      "reset"
#define KEY_BYPASS_TIMEOUT             "bypass_ms"

/* ============================================================================
 * Internal state
 * ========================================================================== */

static bool s_initialized = false;
static VoltageConfig_t s_config;

/* ============================================================================
 * Internal helpers
 * ========================================================================== */

static void voltageConfigSetDefaults(void)
{
    s_config.warningLowV =
        VOLTAGE_CONFIG_DEFAULT_WARNING_LOW_V;

    s_config.warningHighV =
        VOLTAGE_CONFIG_DEFAULT_WARNING_HIGH_V;

    s_config.cutoffBypassV =
        VOLTAGE_CONFIG_DEFAULT_CUTOFF_BYPASS_V;

    s_config.criticalV =
        VOLTAGE_CONFIG_DEFAULT_CRITICAL_V;

    s_config.resetV =
        VOLTAGE_CONFIG_DEFAULT_RESET_V;

    s_config.bypassTimeoutMs =
        VOLTAGE_CONFIG_DEFAULT_BYPASS_TIMEOUT_MS;
}

/* ============================================================================
 * Public API
 * ========================================================================== */

NfwStatus_t voltageConfigValidate(
    const VoltageConfig_t *config)
{
    if (config == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    /*
     * Absolute voltage boundary.
     */
    if (config->criticalV <
        VOLTAGE_CONFIG_ABSOLUTE_MIN_V)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->warningHighV >
        VOLTAGE_CONFIG_ABSOLUTE_MAX_V)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    /*
     * Threshold ordering:
     *
     * critical < cutoff/bypass <= warning low
     * warning low <= reset
     * reset <= warning high
     */
    if (config->criticalV >=
        config->cutoffBypassV)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->cutoffBypassV >
        config->warningLowV)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->warningLowV >
        config->resetV)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->resetV >
        config->warningHighV)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    /*
     * All configured thresholds must remain inside
     * the absolute firmware voltage range.
     */
    if (config->warningLowV <
        VOLTAGE_CONFIG_ABSOLUTE_MIN_V)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->warningHighV >
        VOLTAGE_CONFIG_ABSOLUTE_MAX_V)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    /*
     * Bypass timeout boundary.
     */
    if (config->bypassTimeoutMs <
        VOLTAGE_CONFIG_ABSOLUTE_MIN_TIMEOUT_MS)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->bypassTimeoutMs >
        VOLTAGE_CONFIG_ABSOLUTE_MAX_TIMEOUT_MS)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    return NFW_STATUS_OK;
}

NfwStatus_t voltageConfigInit(void)
{
    nvs_handle_t handle;
    esp_err_t err;

    uint32_t formatVersion;
    uint32_t bypassTimeoutMs;

    size_t length;

    VoltageConfig_t loadedConfig;

    if (s_initialized)
    {
        return NFW_STATUS_ALREADY_INITIALIZED;
    }

    /*
     * Start with firmware fallback values.
     *
     * These remain active if there is no valid EOL/Gateway
     * configuration in NVS.
     */
    voltageConfigSetDefaults();

    err = nvs_open(
        VOLTAGE_CONFIG_NAMESPACE,
        NVS_READONLY,
        &handle);

    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        s_initialized = true;

        return NFW_STATUS_OK;
    }

    if (err != ESP_OK)
    {
        return NFW_STATUS_ERROR;
    }

    err = nvs_get_u32(
        handle,
        KEY_FORMAT_VERSION,
        &formatVersion);

    if (err != ESP_OK ||
        formatVersion != VOLTAGE_CONFIG_FORMAT_VERSION)
    {
        nvs_close(handle);

        s_initialized = true;

        return NFW_STATUS_OK;
    }

    length = sizeof(loadedConfig.warningLowV);

    err = nvs_get_blob(
        handle,
        KEY_WARNING_LOW,
        &loadedConfig.warningLowV,
        &length);

    if (err != ESP_OK ||
        length != sizeof(loadedConfig.warningLowV))
    {
        nvs_close(handle);

        s_initialized = true;

        return NFW_STATUS_OK;
    }

    length = sizeof(loadedConfig.warningHighV);

    err = nvs_get_blob(
        handle,
        KEY_WARNING_HIGH,
        &loadedConfig.warningHighV,
        &length);

    if (err != ESP_OK ||
        length != sizeof(loadedConfig.warningHighV))
    {
        nvs_close(handle);

        s_initialized = true;

        return NFW_STATUS_OK;
    }

    length = sizeof(loadedConfig.cutoffBypassV);

    err = nvs_get_blob(
        handle,
        KEY_CUTOFF_BYPASS,
        &loadedConfig.cutoffBypassV,
        &length);

    if (err != ESP_OK ||
        length != sizeof(loadedConfig.cutoffBypassV))
    {
        nvs_close(handle);

        s_initialized = true;

        return NFW_STATUS_OK;
    }

    length = sizeof(loadedConfig.criticalV);

    err = nvs_get_blob(
        handle,
        KEY_CRITICAL,
        &loadedConfig.criticalV,
        &length);

    if (err != ESP_OK ||
        length != sizeof(loadedConfig.criticalV))
    {
        nvs_close(handle);

        s_initialized = true;

        return NFW_STATUS_OK;
    }

    length = sizeof(loadedConfig.resetV);

    err = nvs_get_blob(
        handle,
        KEY_RESET,
        &loadedConfig.resetV,
        &length);

    if (err != ESP_OK ||
        length != sizeof(loadedConfig.resetV))
    {
        nvs_close(handle);

        s_initialized = true;

        return NFW_STATUS_OK;
    }

    length = sizeof(bypassTimeoutMs);

    err = nvs_get_u32(
        handle,
        KEY_BYPASS_TIMEOUT,
        &bypassTimeoutMs);

    nvs_close(handle);

    if (err != ESP_OK)
    {
        s_initialized = true;

        return NFW_STATUS_OK;
    }

    loadedConfig.bypassTimeoutMs = bypassTimeoutMs;

    /*
     * Never accept invalid or unsafe NVS configuration.
     *
     * If invalid, the firmware fallback values remain active.
     */
    if (voltageConfigValidate(&loadedConfig) != NFW_STATUS_OK)
    {
        s_initialized = true;

        return NFW_STATUS_OK;
    }

    s_config = loadedConfig;

    s_initialized = true;

    return NFW_STATUS_OK;
}

NfwStatus_t voltageConfigLoad(
    VoltageConfig_t *config)
{
    if (config == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    *config = s_config;

    return NFW_STATUS_OK;
}

NfwStatus_t voltageConfigGet(
    VoltageConfig_t *config)
{
    return voltageConfigLoad(config);
}

NfwStatus_t voltageConfigSave(
    const VoltageConfig_t *config)
{
    nvs_handle_t handle;
    esp_err_t err;

    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    /*
     * This validation applies equally to:
     *
     * - EOL factory provisioning
     * - Gateway configuration
     */
    if (voltageConfigValidate(config) != NFW_STATUS_OK)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    err = nvs_open(
        VOLTAGE_CONFIG_NAMESPACE,
        NVS_READWRITE,
        &handle);

    if (err != ESP_OK)
    {
        return NFW_STATUS_ERROR;
    }

    err = nvs_set_u32(
        handle,
        KEY_FORMAT_VERSION,
        VOLTAGE_CONFIG_FORMAT_VERSION);

    if (err == ESP_OK)
    {
        err = nvs_set_blob(
            handle,
            KEY_WARNING_LOW,
            &config->warningLowV,
            sizeof(config->warningLowV));
    }

    if (err == ESP_OK)
    {
        err = nvs_set_blob(
            handle,
            KEY_WARNING_HIGH,
            &config->warningHighV,
            sizeof(config->warningHighV));
    }

    if (err == ESP_OK)
    {
        err = nvs_set_blob(
            handle,
            KEY_CUTOFF_BYPASS,
            &config->cutoffBypassV,
            sizeof(config->cutoffBypassV));
    }

    if (err == ESP_OK)
    {
        err = nvs_set_blob(
            handle,
            KEY_CRITICAL,
            &config->criticalV,
            sizeof(config->criticalV));
    }

    if (err == ESP_OK)
    {
        err = nvs_set_blob(
            handle,
            KEY_RESET,
            &config->resetV,
            sizeof(config->resetV));
    }

    if (err == ESP_OK)
    {
        err = nvs_set_u32(
            handle,
            KEY_BYPASS_TIMEOUT,
            config->bypassTimeoutMs);
    }

    if (err == ESP_OK)
    {
        err = nvs_commit(handle);
    }

    nvs_close(handle);

    if (err != ESP_OK)
    {
        return NFW_STATUS_ERROR;
    }

    /*
     * New configuration becomes active immediately.
     */
    s_config = *config;

    return NFW_STATUS_OK;
}