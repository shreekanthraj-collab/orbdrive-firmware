/**
 * @file ina226.c
 * @brief Orb Drive INA226 current and voltage monitor implementation.
 *
 * The shunt resistance and INA226 calibration may be supplied by EOL/NVS.
 * If calibrationRegister is zero, calibration is calculated from the
 * configured shunt resistance and maximum expected current.
 */

#include "ina226.h"

#include <math.h>
#include <stddef.h>

#include "nfw_i2c.h"

/* INA226 register map */
#define INA226_REG_CONFIGURATION       (0x00U)
#define INA226_REG_SHUNT_VOLTAGE       (0x01U)
#define INA226_REG_BUS_VOLTAGE         (0x02U)
#define INA226_REG_POWER               (0x03U)
#define INA226_REG_CURRENT             (0x04U)
#define INA226_REG_CALIBRATION         (0x05U)
#define INA226_REG_MANUFACTURER_ID     (0xFEU)

/* INA226 conversion constants */
#define INA226_SHUNT_VOLTAGE_LSB_V     (0.0000025f)
#define INA226_BUS_VOLTAGE_LSB_V       (0.00125f)
#define INA226_CALIBRATION_CONSTANT    (0.00512f)

/* INA226 shunt-voltage full-scale is +/-81.92 mV. */
#define INA226_SHUNT_VOLTAGE_MAX_V     (0.08192f)

/*
 * Configuration:
 *   16-sample averaging
 *   1.1 ms bus-voltage conversion
 *   1.1 ms shunt-voltage conversion
 *   continuous shunt + bus measurement
 */
#define INA226_CONFIGURATION_DEFAULT   (0x4527U)

typedef struct
{
    bool initialized;
    uint8_t address;
    float shuntResistanceOhm;
    float currentLsbA;
    uint16_t calibrationRegister;
} Ina226State_t;

static Ina226State_t s_ina226 = {0};

/* ============================================================================
 * Internal register helpers
 * ========================================================================== */

static NfwStatus_t ina226WriteRegister(
    uint8_t reg,
    uint16_t value)
{
    uint8_t data[3];

    data[0] = reg;
    data[1] = (uint8_t)((value >> 8U) & 0xFFU);
    data[2] = (uint8_t)(value & 0xFFU);

    return nfwI2cWrite(
        s_ina226.address,
        data,
        sizeof(data));
}

static NfwStatus_t ina226ReadRegister(
    uint8_t reg,
    uint16_t *value)
{
    uint8_t registerAddress;
    uint8_t data[2];
    NfwStatus_t status;

    if (value == NULL) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    registerAddress = reg;

    status = nfwI2cWriteRead(
        s_ina226.address,
        &registerAddress,
        1U,
        data,
        sizeof(data));

    if (status != NFW_STATUS_OK) {
        return status;
    }

    *value = (uint16_t)(
        ((uint16_t)data[0] << 8U) |
        (uint16_t)data[1]);

    return NFW_STATUS_OK;
}

static NfwStatus_t ina226ReadSignedRegister(
    uint8_t reg,
    int16_t *value)
{
    uint16_t raw;
    NfwStatus_t status;

    if (value == NULL) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    status = ina226ReadRegister(reg, &raw);

    if (status != NFW_STATUS_OK) {
        return status;
    }

    *value = (int16_t)raw;

    return NFW_STATUS_OK;
}

/* ============================================================================
 * Public API
 * ========================================================================== */

NfwStatus_t ina226Init(
    const Ina226Config_t *config)
{
    Ina226Config_t localConfig;
    float currentLsb;
    float calibrationFloat;
    uint32_t calibration;

    if (s_ina226.initialized) {
        return NFW_STATUS_ALREADY_INITIALIZED;
    }

    if (config == NULL) {
        localConfig.i2cAddress =
            INA226_DEFAULT_I2C_ADDRESS;

        localConfig.shuntResistanceOhm =
            INA226_DEFAULT_RSHUNT_OHM;

        localConfig.maximumExpectedCurrentA =
            INA226_DEFAULT_MAX_CURRENT_A;

        localConfig.calibrationRegister = 0U;

        config = &localConfig;
    }

    if (config->i2cAddress > 0x7FU ||
        config->shuntResistanceOhm <= 0.0f ||
        config->maximumExpectedCurrentA <= 0.0f) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if ((config->maximumExpectedCurrentA *
         config->shuntResistanceOhm) >
        INA226_SHUNT_VOLTAGE_MAX_V) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!nfwI2cDevicePresent(config->i2cAddress)) {
        return NFW_STATUS_NOT_FOUND;
    }

    /*
     * Use the EOL/NVS calibration when supplied.
     */
    if (config->calibrationRegister != 0U) {

        calibration =
            config->calibrationRegister;

        currentLsb =
            INA226_CALIBRATION_CONSTANT /
            ((float)calibration *
             config->shuntResistanceOhm);

        if (!isfinite(currentLsb) ||
            currentLsb <= 0.0f) {
            return NFW_STATUS_INVALID_ARGUMENT;
        }

    } else {

        /*
         * No EOL calibration supplied.
         *
         * Calculate a calibration value from the configured maximum
         * expected current. This is useful for commissioning/factory setup,
         * but the production EOL process should save the resulting value.
         */
        currentLsb =
            config->maximumExpectedCurrentA /
            32768.0f;

        calibrationFloat =
            INA226_CALIBRATION_CONSTANT /
            (currentLsb *
             config->shuntResistanceOhm);

        if (!isfinite(calibrationFloat) ||
            calibrationFloat < 1.0f ||
            calibrationFloat > 65534.0f) {
            return NFW_STATUS_INVALID_ARGUMENT;
        }

        calibration =
            (uint32_t)(calibrationFloat + 0.5f);

        if (calibration == 0U ||
            calibration > 65534U) {
            return NFW_STATUS_INVALID_ARGUMENT;
        }

        /*
         * Recalculate Current_LSB using the exact integer calibration value
         * written to the INA226.
         */
        currentLsb =
            INA226_CALIBRATION_CONSTANT /
            ((float)calibration *
             config->shuntResistanceOhm);
    }

    s_ina226.address =
        config->i2cAddress;

    s_ina226.shuntResistanceOhm =
        config->shuntResistanceOhm;

    s_ina226.calibrationRegister =
        (uint16_t)calibration;

    s_ina226.currentLsbA =
        currentLsb;

    if (ina226WriteRegister(
            INA226_REG_CALIBRATION,
            s_ina226.calibrationRegister) !=
        NFW_STATUS_OK) {

        s_ina226 = (Ina226State_t){0};

        return NFW_STATUS_ERROR;
    }

    if (ina226WriteRegister(
            INA226_REG_CONFIGURATION,
            INA226_CONFIGURATION_DEFAULT) !=
        NFW_STATUS_OK) {

        s_ina226 = (Ina226State_t){0};

        return NFW_STATUS_ERROR;
    }

    s_ina226.initialized = true;

    return NFW_STATUS_OK;
}

NfwStatus_t ina226ReadBusVoltage(
    float *voltageV)
{
    uint16_t raw;
    NfwStatus_t status;

    if (voltageV == NULL) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!s_ina226.initialized) {
        return NFW_STATUS_INVALID_STATE;
    }

    status = ina226ReadRegister(
        INA226_REG_BUS_VOLTAGE,
        &raw);

    if (status != NFW_STATUS_OK) {
        return status;
    }

    *voltageV =
        (float)raw *
        INA226_BUS_VOLTAGE_LSB_V;

    return NFW_STATUS_OK;
}

NfwStatus_t ina226ReadShuntVoltage(
    float *voltageV)
{
    int16_t raw;
    NfwStatus_t status;

    if (voltageV == NULL) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!s_ina226.initialized) {
        return NFW_STATUS_INVALID_STATE;
    }

    status = ina226ReadSignedRegister(
        INA226_REG_SHUNT_VOLTAGE,
        &raw);

    if (status != NFW_STATUS_OK) {
        return status;
    }

    *voltageV =
        (float)raw *
        INA226_SHUNT_VOLTAGE_LSB_V;

    return NFW_STATUS_OK;
}

NfwStatus_t ina226ReadCurrent(
    float *currentA)
{
    int16_t raw;
    NfwStatus_t status;

    if (currentA == NULL) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!s_ina226.initialized) {
        return NFW_STATUS_INVALID_STATE;
    }

    status = ina226ReadSignedRegister(
        INA226_REG_CURRENT,
        &raw);

    if (status != NFW_STATUS_OK) {
        return status;
    }

    *currentA =
        (float)raw *
        s_ina226.currentLsbA;

    return NFW_STATUS_OK;
}

NfwStatus_t ina226ReadPower(
    float *powerW)
{
    uint16_t raw;
    NfwStatus_t status;

    if (powerW == NULL) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!s_ina226.initialized) {
        return NFW_STATUS_INVALID_STATE;
    }

    status = ina226ReadRegister(
        INA226_REG_POWER,
        &raw);

    if (status != NFW_STATUS_OK) {
        return status;
    }

    *powerW =
        (float)raw *
        (25.0f * s_ina226.currentLsbA);

    return NFW_STATUS_OK;
}

NfwStatus_t ina226ReadMeasurement(
    Ina226Measurement_t *measurement)
{
    NfwStatus_t status;

    if (measurement == NULL) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!s_ina226.initialized) {
        return NFW_STATUS_INVALID_STATE;
    }

    status = ina226ReadBusVoltage(
        &measurement->busVoltageV);

    if (status != NFW_STATUS_OK) {
        return status;
    }

    status = ina226ReadShuntVoltage(
        &measurement->shuntVoltageV);

    if (status != NFW_STATUS_OK) {
        return status;
    }

    status = ina226ReadCurrent(
        &measurement->currentA);

    if (status != NFW_STATUS_OK) {
        return status;
    }

    status = ina226ReadPower(
        &measurement->powerW);

    if (status != NFW_STATUS_OK) {
        return status;
    }

    return NFW_STATUS_OK;
}

bool ina226IsReady(void)
{
    uint16_t manufacturerId;

    if (!s_ina226.initialized) {
        return false;
    }

    return (
        ina226ReadRegister(
            INA226_REG_MANUFACTURER_ID,
            &manufacturerId) ==
        NFW_STATUS_OK);
}