/**
 * @file oc_config.c
 * @brief Persistent EOL/Gateway-configurable over-current limits.
 */

#include "oc_config.h"

#include <stdbool.h>
#include <stddef.h>

#include "nvs.h"
#include "nvs_flash.h"

/* ============================================================================
 * NVS configuration
 * ========================================================================== */

#define OC_CONFIG_NAMESPACE        "orb_oc"
#define OC_CONFIG_FORMAT_VERSION   (1U)

#define KEY_FORMAT_VERSION         "fmt_ver"
#define KEY_MIN_CURRENT            "min_current"
#define KEY_MAX_CURRENT            "max_current"

/* ============================================================================
 * Internal state
 * ========================================================================== */

static bool s_initialized = false;
static OcCurrentConfig_t s_config;

/* ============================================================================
 * Internal helpers
 * ========================================================================== */

static void ocConfigSetDefaults(void)
{
    s_config.minimumSafeCurrentA =
        OC_CONFIG_DEFAULT_MIN_CURRENT_A;

    s_config.maximumSafeCurrentA =
        OC_CONFIG_DEFAULT_MAX_CURRENT_A;
}

/* ============================================================================
 * Public API
 * ========================================================================== */

NfwStatus_t ocConfigValidate(
    const OcCurrentConfig_t *config)
{
    if (config == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->minimumSafeCurrentA <
        OC_CONFIG_ABSOLUTE_MIN_CURRENT_A)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->maximumSafeCurrentA >
        OC_CONFIG_ABSOLUTE_MAX_CURRENT_A)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->maximumSafeCurrentA <
        config->minimumSafeCurrentA)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    return NFW_STATUS_OK;
}

NfwStatus_t ocConfigInit(void)
{
    nvs_handle_t handle;
    esp_err_t err;

    uint32_t formatVersion;

    size_t length;

    float minimumCurrent;
    float maximumCurrent;

    OcCurrentConfig_t loadedConfig;

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
    ocConfigSetDefaults();

    err = nvs_open(
        OC_CONFIG_NAMESPACE,
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
        formatVersion != OC_CONFIG_FORMAT_VERSION)
    {
        nvs_close(handle);

        s_initialized = true;

        return NFW_STATUS_OK;
    }

    length = sizeof(minimumCurrent);

    err = nvs_get_blob(
        handle,
        KEY_MIN_CURRENT,
        &minimumCurrent,
        &length);

    if (err != ESP_OK ||
        length != sizeof(minimumCurrent))
    {
        nvs_close(handle);

        s_initialized = true;

        return NFW_STATUS_OK;
    }

    length = sizeof(maximumCurrent);

    err = nvs_get_blob(
        handle,
        KEY_MAX_CURRENT,
        &maximumCurrent,
        &length);

    nvs_close(handle);

    if (err != ESP_OK ||
        length != sizeof(maximumCurrent))
    {
        s_initialized = true;

        return NFW_STATUS_OK;
    }

    loadedConfig.minimumSafeCurrentA = minimumCurrent;
    loadedConfig.maximumSafeCurrentA = maximumCurrent;

    /*
     * Never accept an invalid or unsafe NVS configuration.
     *
     * If invalid, keep firmware fallback values.
     */
    if (ocConfigValidate(&loadedConfig) != NFW_STATUS_OK)
    {
        s_initialized = true;

        return NFW_STATUS_OK;
    }

    s_config = loadedConfig;

    s_initialized = true;

    return NFW_STATUS_OK;
}

NfwStatus_t ocConfigLoad(
    OcCurrentConfig_t *config)
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

NfwStatus_t ocConfigGet(
    OcCurrentConfig_t *config)
{
    return ocConfigLoad(config);
}

NfwStatus_t ocConfigSave(
    const OcCurrentConfig_t *config)
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
    if (ocConfigValidate(config) != NFW_STATUS_OK)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    err = nvs_open(
        OC_CONFIG_NAMESPACE,
        NVS_READWRITE,
        &handle);

    if (err != ESP_OK)
    {
        return NFW_STATUS_ERROR;
    }

    err = nvs_set_u32(
        handle,
        KEY_FORMAT_VERSION,
        OC_CONFIG_FORMAT_VERSION);

    if (err == ESP_OK)
    {
        err = nvs_set_blob(
            handle,
            KEY_MIN_CURRENT,
            &config->minimumSafeCurrentA,
            sizeof(config->minimumSafeCurrentA));
    }

    if (err == ESP_OK)
    {
        err = nvs_set_blob(
            handle,
            KEY_MAX_CURRENT,
            &config->maximumSafeCurrentA,
            sizeof(config->maximumSafeCurrentA));
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