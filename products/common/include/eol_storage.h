/**
 * @file eol_storage.h
 * @brief Persistent EOL calibration and hardware configuration contract.
 *
 * EOL data is stored in ESP-IDF NVS by the implementation layer.
 * This header contains only the product-level data/API contract.
 *
 * The INA226 shunt resistance is adjustable through EOL/NVS.
 * Factory/default value: 0.010 ohm.
 */

#ifndef EOL_STORAGE_H
#define EOL_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "nfw_status.h"

/* ============================================================================
 * EOL defaults
 * ========================================================================== */

#define EOL_STORAGE_DEFAULT_RSHUNT_OHM       (0.010f)
#define EOL_STORAGE_DEFAULT_BOARD_VERSION    "1.0.0"

/*
 * Configuration format version.
 *
 * Increment this when the persistent EOL data structure or its meaning
 * changes incompatibly.
 */
#define EOL_STORAGE_FORMAT_VERSION           (1U)

/* ============================================================================
 * Persistent EOL configuration
 * ========================================================================== */

typedef struct
{
    uint32_t formatVersion;

    /*
     * True only after a successful EOL validation/calibration sequence.
     */
    bool eolValid;

    /*
     * Board identity/configuration used when EOL was completed.
     */
    char boardVersion[16];

    /*
     * INA226 shunt resistance used for calibration.
     */
    float shuntResistanceOhm;

    /*
     * INA226 calibration register calculated during EOL.
     */
    uint16_t ina226Calibration;

    /*
     * Hardware/configuration fingerprint captured during EOL.
     *
     * A change in the relevant hardware configuration must invalidate EOL.
     */
    uint32_t hardwareFingerprint;
} EolStorageData_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

/**
 * @brief Initialize the EOL storage subsystem.
 *
 * Loads the persistent EOL record from NVS when available.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t eolStorageInit(void);

/**
 * @brief Load the currently stored EOL configuration.
 *
 * @param data Destination for the stored configuration.
 *
 * @return NFW_STATUS_OK when a valid record is loaded.
 * @return NFW_STATUS_NOT_FOUND when no EOL record exists.
 */
NfwStatus_t eolStorageLoad(
    EolStorageData_t *data);

/**
 * @brief Save a completed EOL configuration to NVS.
 *
 * The supplied configuration must contain the calibration values produced
 * by the EOL procedure.
 *
 * @param data EOL configuration to persist.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t eolStorageSave(
    const EolStorageData_t *data);

/**
 * @brief Invalidate the stored EOL validation.
 *
 * This is used when hardware/configuration changes require EOL to be run
 * again.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t eolStorageInvalidate(void);

/**
 * @brief Check whether a valid EOL record is currently stored.
 *
 * @return true only when the stored EOL record is marked valid.
 */
bool eolStorageIsValid(void);

/**
 * @brief Get the currently stored shunt resistance.
 *
 * @param resistanceOhm Destination for the resistance value.
 *
 * @return NFW_STATUS_OK when available.
 */
NfwStatus_t eolStorageGetShuntResistance(
    float *resistanceOhm);

/**
 * @brief Get the stored INA226 calibration register.
 *
 * @param calibration Destination for the calibration value.
 *
 * @return NFW_STATUS_OK when available.
 */
NfwStatus_t eolStorageGetIna226Calibration(
    uint16_t *calibration);

/**
 * @brief Calculate the hardware/configuration fingerprint.
 *
 * The fingerprint represents the board configuration used by EOL.
 * The supplied shunt resistance is included so that changing Rshunt
 * requires EOL calibration again.
 *
 * @param shuntResistanceOhm Current Rshunt value.
 *
 * @return Non-zero hardware/configuration fingerprint.
 */
uint32_t eolStorageCalculateHardwareFingerprint(
    float shuntResistanceOhm);

/**
 * @brief Validate the current hardware/configuration fingerprint.
 *
 * If the supplied fingerprint differs from the fingerprint captured during
 * EOL, the stored EOL state is invalidated.
 *
 * @param currentFingerprint Current hardware/configuration fingerprint.
 *
 * @return NFW_STATUS_OK when the stored EOL configuration matches.
 * @return NFW_STATUS_INVALID_STATE when EOL must be performed again.
 */
NfwStatus_t eolStorageValidateHardware(
    uint32_t currentFingerprint);

#ifdef __cplusplus
}
#endif

#endif /* EOL_STORAGE_H */