/**
 * @file ina226.h
 * @brief Orb Drive INA226 current and voltage monitor.
 */

#ifndef INA226_H
#define INA226_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "nfw_status.h"

/* ============================================================================
 * INA226 configuration
 * ========================================================================== */

#define INA226_DEFAULT_I2C_ADDRESS      (0x40U)
#define INA226_DEFAULT_RSHUNT_OHM       (0.010f)
#define INA226_DEFAULT_MAX_CURRENT_A    (8.0f)

/* ============================================================================
 * Measurement data
 * ========================================================================== */

typedef struct
{
    float busVoltageV;
    float shuntVoltageV;
    float currentA;
    float powerW;
} Ina226Measurement_t;

/* ============================================================================
 * Configuration
 * ========================================================================== */

typedef struct
{
    uint8_t i2cAddress;
    float shuntResistanceOhm;
    float maximumExpectedCurrentA;

    /*
     * Optional EOL calibration value.
     *
     * When zero, ina226Init() calculates the calibration register from
     * shuntResistanceOhm and maximumExpectedCurrentA.
     *
     * When non-zero, the EOL-provided calibration register is used.
     */
    uint16_t calibrationRegister;
} Ina226Config_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

/**
 * @brief Initialize the INA226.
 *
 * @param config INA226 configuration.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t ina226Init(const Ina226Config_t *config);

/**
 * @brief Read bus voltage.
 *
 * @param voltageV Pointer receiving bus voltage in volts.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t ina226ReadBusVoltage(float *voltageV);

/**
 * @brief Read shunt voltage.
 *
 * @param voltageV Pointer receiving shunt voltage in volts.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t ina226ReadShuntVoltage(float *voltageV);

/**
 * @brief Read current.
 *
 * @param currentA Pointer receiving current in amperes.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t ina226ReadCurrent(float *currentA);

/**
 * @brief Read power.
 *
 * @param powerW Pointer receiving power in watts.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t ina226ReadPower(float *powerW);

/**
 * @brief Read all INA226 measurements.
 *
 * @param measurement Pointer receiving measurement data.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t ina226ReadMeasurement(
    Ina226Measurement_t *measurement);

/**
 * @brief Check whether the INA226 is responding.
 *
 * @return true when the device responds correctly.
 */
bool ina226IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* INA226_H */