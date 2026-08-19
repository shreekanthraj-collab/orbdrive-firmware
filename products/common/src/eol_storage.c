/**
 * @file eol_storage.c
 * @brief Persistent EOL calibration/configuration storage using ESP-IDF NVS.
 */

#include "eol_storage.h"

#include <string.h>

#include "nvs.h"
#include "nvs_flash.h"

#include "orb_drive_main.h"

#define EOL_STORAGE_NAMESPACE        "orb_eol"

#define KEY_FORMAT_VERSION           "fmt_ver"
#define KEY_EOL_VALID                "eol_valid"
#define KEY_BOARD_VERSION            "board_ver"
#define KEY_RSHUNT_OHM               "rshunt"
#define KEY_INA226_CALIBRATION       "ina_cal"
#define KEY_HW_FINGERPRINT           "hw_fp"

/*
 * FNV-1a 32-bit constants.
 *
 * The fingerprint is deliberately deterministic and independent of
 * compiler/platform padding.
 */
#define EOL_FP_OFFSET_BASIS          (2166136261UL)
#define EOL_FP_PRIME                 (16777619UL)

static bool s_initialized = false;
static EolStorageData_t s_data;

/* ============================================================================
 * Internal helpers
 * ========================================================================== */

static void eolStorageSetDefaults(void)
{
    memset(&s_data, 0, sizeof(s_data));

    s_data.formatVersion = EOL_STORAGE_FORMAT_VERSION;
    s_data.eolValid = false;

    strncpy(
        s_data.boardVersion,
        EOL_STORAGE_DEFAULT_BOARD_VERSION,
        sizeof(s_data.boardVersion) - 1U);

    s_data.boardVersion[
        sizeof(s_data.boardVersion) - 1U] = '\0';

    s_data.shuntResistanceOhm =
        EOL_STORAGE_DEFAULT_RSHUNT_OHM;
}

static uint32_t eolFingerprintAddByte(
    uint32_t hash,
    uint8_t value)
{
    hash ^= (uint32_t)value;
    hash *= EOL_FP_PRIME;
    return hash;
}

static uint32_t eolFingerprintAddU32(
    uint32_t hash,
    uint32_t value)
{
    hash = eolFingerprintAddByte(
        hash,
        (uint8_t)(value & 0xFFU));

    hash = eolFingerprintAddByte(
        hash,
        (uint8_t)((value >> 8U) & 0xFFU));

    hash = eolFingerprintAddByte(
        hash,
        (uint8_t)((value >> 16U) & 0xFFU));

    hash = eolFingerprintAddByte(
        hash,
        (uint8_t)((value >> 24U) & 0xFFU));

    return hash;
}

static uint32_t eolFingerprintAddFloat(
    uint32_t hash,
    float value)
{
    uint32_t raw;

    memcpy(&raw, &value, sizeof(raw));

    return eolFingerprintAddU32(hash, raw);
}

static uint32_t eolFingerprintAddString(
    uint32_t hash,
    const char *value)
{
    if (value == NULL) {
        return eolFingerprintAddByte(hash, 0U);
    }

    while (*value != '\0') {
        hash = eolFingerprintAddByte(
            hash,
            (uint8_t)*value);

        value++;
    }

    return eolFingerprintAddByte(hash, 0U);
}

static NfwStatus_t eolStorageLoadInternal(void)
{
    nvs_handle_t handle;
    esp_err_t err;
    size_t length;
    size_t blobLength;
    uint8_t eolValid;

    err = nvs_open(
        EOL_STORAGE_NAMESPACE,
        NVS_READONLY,
        &handle);

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return NFW_STATUS_NOT_FOUND;
    }

    if (err != ESP_OK) {
        return NFW_STATUS_ERROR;
    }

    eolStorageSetDefaults();

    err = nvs_get_u32(
        handle,
        KEY_FORMAT_VERSION,
        &s_data.formatVersion);

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        nvs_close(handle);
        eolStorageSetDefaults();
        return NFW_STATUS_NOT_FOUND;
    }

    if (err != ESP_OK) {
        nvs_close(handle);
        eolStorageSetDefaults();
        return NFW_STATUS_ERROR;
    }

    if (s_data.formatVersion != EOL_STORAGE_FORMAT_VERSION) {
        nvs_close(handle);
        eolStorageSetDefaults();
        return NFW_STATUS_INVALID_STATE;
    }

    err = nvs_get_u8(
        handle,
        KEY_EOL_VALID,
        &eolValid);

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        nvs_close(handle);
        eolStorageSetDefaults();
        return NFW_STATUS_NOT_FOUND;
    }

    if (err != ESP_OK) {
        nvs_close(handle);
        eolStorageSetDefaults();
        return NFW_STATUS_ERROR;
    }

    s_data.eolValid = (eolValid != 0U);

    length = sizeof(s_data.boardVersion);

    err = nvs_get_str(
        handle,
        KEY_BOARD_VERSION,
        s_data.boardVersion,
        &length);

    if (err != ESP_OK) {
        nvs_close(handle);
        eolStorageSetDefaults();
        return NFW_STATUS_ERROR;
    }

    blobLength = sizeof(s_data.shuntResistanceOhm);

    err = nvs_get_blob(
        handle,
        KEY_RSHUNT_OHM,
        &s_data.shuntResistanceOhm,
        &blobLength);

    if (err != ESP_OK ||
        blobLength != sizeof(s_data.shuntResistanceOhm)) {
        nvs_close(handle);
        eolStorageSetDefaults();
        return NFW_STATUS_ERROR;
    }

    err = nvs_get_u16(
        handle,
        KEY_INA226_CALIBRATION,
        &s_data.ina226Calibration);

    if (err != ESP_OK) {
        nvs_close(handle);
        eolStorageSetDefaults();
        return NFW_STATUS_ERROR;
    }

    err = nvs_get_u32(
        handle,
        KEY_HW_FINGERPRINT,
        &s_data.hardwareFingerprint);

    if (err != ESP_OK) {
        nvs_close(handle);
        eolStorageSetDefaults();
        return NFW_STATUS_ERROR;
    }

    nvs_close(handle);

    if (s_data.boardVersion[0] == '\0' ||
        s_data.shuntResistanceOhm <= 0.0f) {
        eolStorageSetDefaults();
        return NFW_STATUS_INVALID_STATE;
    }

    return NFW_STATUS_OK;
}

/* ============================================================================
 * Public API
 * ========================================================================== */

NfwStatus_t eolStorageInit(void)
{
    esp_err_t err;
    NfwStatus_t loadStatus;

    if (s_initialized) {
        return NFW_STATUS_ALREADY_INITIALIZED;
    }

    err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /*
         * Never erase NVS automatically during normal firmware startup.
         * EOL/calibration data must not be destroyed by boot-time recovery.
         */
        return NFW_STATUS_ERROR;
    }

    if (err != ESP_OK) {
        return NFW_STATUS_ERROR;
    }

    eolStorageSetDefaults();
    s_initialized = true;

    /*
     * Load the persistent EOL record immediately so that the in-memory
     * state reflects the saved state after every reboot.
     *
     * A missing record is normal for a new/uncommissioned device.
     */
    loadStatus = eolStorageLoadInternal();

    if (loadStatus == NFW_STATUS_NOT_FOUND) {
        eolStorageSetDefaults();
        return NFW_STATUS_OK;
    }

    if (loadStatus != NFW_STATUS_OK) {
        eolStorageSetDefaults();
        return loadStatus;
    }

    return NFW_STATUS_OK;
}

NfwStatus_t eolStorageLoad(
    EolStorageData_t *data)
{
    NfwStatus_t status;

    if (data == NULL) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!s_initialized) {
        return NFW_STATUS_INVALID_STATE;
    }

    status = eolStorageLoadInternal();

    if (status != NFW_STATUS_OK) {
        return status;
    }

    *data = s_data;

    return NFW_STATUS_OK;
}

NfwStatus_t eolStorageSave(
    const EolStorageData_t *data)
{
    nvs_handle_t handle;
    esp_err_t err;
    uint8_t eolValid;

    if (data == NULL) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!s_initialized) {
        return NFW_STATUS_INVALID_STATE;
    }

    if (data->formatVersion != EOL_STORAGE_FORMAT_VERSION ||
        data->shuntResistanceOhm <= 0.0f ||
        data->boardVersion[0] == '\0') {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    err = nvs_open(
        EOL_STORAGE_NAMESPACE,
        NVS_READWRITE,
        &handle);

    if (err != ESP_OK) {
        return NFW_STATUS_ERROR;
    }

    eolValid = data->eolValid ? 1U : 0U;

    err = nvs_set_u32(
        handle,
        KEY_FORMAT_VERSION,
        data->formatVersion);

    if (err == ESP_OK) {
        err = nvs_set_u8(
            handle,
            KEY_EOL_VALID,
            eolValid);
    }

    if (err == ESP_OK) {
        err = nvs_set_str(
            handle,
            KEY_BOARD_VERSION,
            data->boardVersion);
    }

    if (err == ESP_OK) {
        err = nvs_set_blob(
            handle,
            KEY_RSHUNT_OHM,
            &data->shuntResistanceOhm,
            sizeof(data->shuntResistanceOhm));
    }

    if (err == ESP_OK) {
        err = nvs_set_u16(
            handle,
            KEY_INA226_CALIBRATION,
            data->ina226Calibration);
    }

    if (err == ESP_OK) {
        err = nvs_set_u32(
            handle,
            KEY_HW_FINGERPRINT,
            data->hardwareFingerprint);
    }

    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);

    if (err != ESP_OK) {
        return NFW_STATUS_ERROR;
    }

    s_data = *data;

    s_data.boardVersion[
        sizeof(s_data.boardVersion) - 1U] = '\0';

    return NFW_STATUS_OK;
}

NfwStatus_t eolStorageInvalidate(void)
{
    EolStorageData_t data;

    if (!s_initialized) {
        return NFW_STATUS_INVALID_STATE;
    }

    data = s_data;
    data.formatVersion = EOL_STORAGE_FORMAT_VERSION;
    data.eolValid = false;

    return eolStorageSave(&data);
}

bool eolStorageIsValid(void)
{
    if (!s_initialized) {
        return false;
    }

    return s_data.eolValid;
}

NfwStatus_t eolStorageGetShuntResistance(
    float *resistanceOhm)
{
    if (resistanceOhm == NULL) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!s_initialized) {
        return NFW_STATUS_INVALID_STATE;
    }

    *resistanceOhm = s_data.shuntResistanceOhm;

    return NFW_STATUS_OK;
}

NfwStatus_t eolStorageGetIna226Calibration(
    uint16_t *calibration)
{
    if (calibration == NULL) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!s_initialized) {
        return NFW_STATUS_INVALID_STATE;
    }

    *calibration = s_data.ina226Calibration;

    return NFW_STATUS_OK;
}

uint32_t eolStorageCalculateHardwareFingerprint(
    float shuntResistanceOhm)
{
    uint32_t hash = EOL_FP_OFFSET_BASIS;

    /*
     * Include the board identity.
     */
    hash = eolFingerprintAddString(
        hash,
        ORB_BOARD_NAME);

    hash = eolFingerprintAddString(
        hash,
        ORB_BOARD_VERSION);

    /*
     * Include the firmware EOL data format.
     */
    hash = eolFingerprintAddU32(
        hash,
        EOL_STORAGE_FORMAT_VERSION);

    /*
     * Include INA226 hardware configuration.
     */
    hash = eolFingerprintAddU32(
        hash,
        ORB_INA226_I2C_ADDRESS);

    hash = eolFingerprintAddFloat(
        hash,
        shuntResistanceOhm);

    hash = eolFingerprintAddFloat(
        hash,
        ORB_INA226_MAX_CURRENT_A);

    /*
     * Include I2C physical mapping.
     */
    hash = eolFingerprintAddU32(
        hash,
        ORB_I2C_PORT);

    hash = eolFingerprintAddU32(
        hash,
        ORB_I2C_SDA_GPIO);

    hash = eolFingerprintAddU32(
        hash,
        ORB_I2C_SCL_GPIO);

    hash = eolFingerprintAddU32(
        hash,
        ORB_I2C_FREQUENCY_HZ);

    /*
     * Include motor/actuator GPIO ownership.
     */
    hash = eolFingerprintAddU32(
        hash,
        ORB_GPIO_MOTOR_POWER_RELAY);

    hash = eolFingerprintAddU32(
        hash,
        ORB_GPIO_MOTOR_FORWARD_RELAY);

    hash = eolFingerprintAddU32(
        hash,
        ORB_GPIO_MOTOR_REVERSE_RELAY);

    hash = eolFingerprintAddU32(
        hash,
        ORB_GPIO_MOTOR_SOFT_START_PWM);

    /*
     * Include relay polarity and motor PWM configuration.
     */
    hash = eolFingerprintAddU32(
        hash,
        ORB_MOTOR_PWM_FREQUENCY_HZ);

    hash = eolFingerprintAddU32(
        hash,
        ORB_MOTOR_PWM_RESOLUTION_BITS);

    /*
     * Keep fingerprint non-zero so zero can never look like a valid
     * calculated fingerprint.
     */
    if (hash == 0U) {
        hash = 1U;
    }

    return hash;
}

NfwStatus_t eolStorageValidateHardware(
    uint32_t currentFingerprint)
{
    if (!s_initialized) {
        return NFW_STATUS_INVALID_STATE;
    }

    if (!s_data.eolValid) {
        return NFW_STATUS_INVALID_STATE;
    }

    if (s_data.hardwareFingerprint != currentFingerprint) {
        (void)eolStorageInvalidate();
        return NFW_STATUS_INVALID_STATE;
    }

    return NFW_STATUS_OK;
}