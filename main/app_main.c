/**
 * @file app_main.c
 * @brief Orb Drive application startup.
 */

#include <stdio.h>

#include "eol_storage.h"
#include "ina226.h"
#include "motor_controller.h"
#include "nfw_i2c.h"
#include "orb_drive_main.h"

/* ============================================================================
 * Application startup
 * ========================================================================== */

void app_main(void)
{
    NfwStatus_t status;
    NfwStatus_t eolStatus;
    NfwStatus_t eolValidationStatus;

    EolStorageData_t eolData;
    NfwI2cConfig_t i2cConfig;
    Ina226Config_t inaConfig;

    float shuntResistanceOhm;
    uint16_t ina226Calibration;
    uint32_t hardwareFingerprint;

    bool eolReady = false;

    printf("\n");
    printf("=====================================\n");
    printf(" Orb Drive Firmware\n");
    printf(" ESP32-S3\n");
    printf(" Board: %s\n", ORB_BOARD_NAME);
    printf(" Version: %s\n", ORB_BOARD_VERSION);
    printf("=====================================\n");

    /* ========================================================================
     * 1. Initialize EOL/NVS storage
     * ====================================================================== */

    status = eolStorageInit();

    if (status != NFW_STATUS_OK) {
        printf("EOL storage initialization FAILED: %d\n",
               (int)status);
        return;
    }

    printf("EOL storage initialization: PASS\n");

    /* ========================================================================
     * 2. Determine current shunt configuration
     * ====================================================================== */

    eolStatus = eolStorageLoad(&eolData);

    if (eolStatus == NFW_STATUS_OK) {
        shuntResistanceOhm = eolData.shuntResistanceOhm;
    } else {
        shuntResistanceOhm = EOL_STORAGE_DEFAULT_RSHUNT_OHM;
        eolData.ina226Calibration = 0U;
    }

    if (shuntResistanceOhm <= 0.0f) {
        shuntResistanceOhm =
            EOL_STORAGE_DEFAULT_RSHUNT_OHM;
    }

    /* ========================================================================
     * 3. Calculate and validate hardware/configuration fingerprint
     * ====================================================================== */

    hardwareFingerprint =
        eolStorageCalculateHardwareFingerprint(
            shuntResistanceOhm);

    eolValidationStatus =
        eolStorageValidateHardware(
            hardwareFingerprint);

    if (eolValidationStatus == NFW_STATUS_OK) {
        eolReady = true;

        printf("EOL validation: PASS\n");
        printf("EOL Rshunt: %.6f ohm\n",
               shuntResistanceOhm);

        status = eolStorageGetIna226Calibration(
            &ina226Calibration);

        if (status != NFW_STATUS_OK ||
            ina226Calibration == 0U) {
            eolReady = false;

            printf("EOL calibration data: INVALID\n");
        }
    } else {
        eolReady = false;
        ina226Calibration = 0U;

        printf("EOL validation: REQUIRED\n");
        printf("Hardware/configuration fingerprint mismatch or no EOL record.\n");
        printf("Stored calibration will NOT be treated as production-valid.\n");
    }

    /* ========================================================================
     * 4. Initialize I2C
     * ====================================================================== */

    i2cConfig.port = ORB_I2C_PORT;
    i2cConfig.sdaPin = ORB_I2C_SDA_GPIO;
    i2cConfig.sclPin = ORB_I2C_SCL_GPIO;
    i2cConfig.frequencyHz = ORB_I2C_FREQUENCY_HZ;

    status = nfwI2cInit(&i2cConfig);

    if (status != NFW_STATUS_OK) {
        printf("I2C initialization FAILED: %d\n",
               (int)status);
        return;
    }

    printf("I2C initialization: PASS\n");
    printf("I2C SDA: GPIO%u\n",
           (unsigned)ORB_I2C_SDA_GPIO);
    printf("I2C SCL: GPIO%u\n",
           (unsigned)ORB_I2C_SCL_GPIO);
    printf("I2C frequency: %u Hz\n",
           (unsigned)ORB_I2C_FREQUENCY_HZ);

    /* ========================================================================
     * 5. Initialize INA226
     * ====================================================================== */

    inaConfig.i2cAddress =
        ORB_INA226_I2C_ADDRESS;

    inaConfig.shuntResistanceOhm =
        shuntResistanceOhm;

    inaConfig.maximumExpectedCurrentA =
        ORB_INA226_MAX_CURRENT_A;

    /*
     * Only use the persisted INA226 calibration when EOL validation passed.
     *
     * When EOL is required, calibrationRegister remains zero and ina226Init()
     * calculates a commissioning/default calibration instead. This does NOT
     * make the unit EOL-valid.
     */
    inaConfig.calibrationRegister =
        eolReady ? ina226Calibration : 0U;

    status = ina226Init(&inaConfig);

    if (status != NFW_STATUS_OK) {
        printf("INA226 initialization FAILED: %d\n",
               (int)status);
        return;
    }

    printf("INA226 initialization: PASS\n");
    printf("INA226 address: 0x%02X\n",
           (unsigned)ORB_INA226_I2C_ADDRESS);
    printf("INA226 Rshunt: %.6f ohm\n",
           shuntResistanceOhm);

    if (eolReady) {
        printf("INA226 calibration source: EOL/NVS\n");
    } else {
        printf("INA226 calibration source: DEFAULT/COMMISSIONING\n");
        printf("WARNING: EOL calibration is REQUIRED.\n");
    }

    /* ========================================================================
     * 6. Initialize motor controller
     * ====================================================================== */

    status = motorControllerInit();

    if (status != NFW_STATUS_OK) {
        printf("Motor controller initialization FAILED: %d\n",
               (int)status);
        return;
    }

    printf("Motor controller initialization: PASS\n");
    printf("Motor state: STOPPED\n");
    printf("Motor PWM: 0%%\n");

    /* ========================================================================
     * Final startup status
     * ====================================================================== */

    printf("-------------------------------------\n");

    if (eolReady) {
        printf("EOL STATUS: VALID\n");
        printf("Startup status: READY\n");
    } else {
        printf("EOL STATUS: REQUIRED\n");
        printf("Startup status: COMMISSIONING REQUIRED\n");
    }

    printf("=====================================\n");
}