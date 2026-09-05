/**
 * @file app_main.c
 * @brief Orb Drive application startup.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/task.h"
#include "esp_ota_ops.h"

#include "eol_storage.h"
#include "ina226.h"
#include "oc_config.h"
#include "voltage_config.h"
#include "lora_transport.h"
#include "motor_controller.h"
#include "nfw_i2c.h"
#include "nfw_gpio.h"
#include "orb_drive_main.h"

#include "ota_wifi_ap.h"
#include "ota_server.h"

/* ============================================================================
 * OTA buzzer
 * ========================================================================== */

#define OTA_BUZZER_ON_LEVEL       (false)
#define OTA_BUZZER_OFF_LEVEL      (true)

static void otaBuzzerInit(void)
{
    NfwGpioConfig_t config;

    config.pin = ORB_GPIO_BUZZER;
    config.direction = NFW_GPIO_DIRECTION_OUTPUT;
    config.pull = NFW_GPIO_PULL_NONE;
    config.initialLevel = OTA_BUZZER_OFF_LEVEL;

    (void)nfwGpioInit(&config);
}

static void otaBuzzerPending(void)
{
    /* Two short beeps indicate a new OTA image is pending verification. */

    (void)nfwGpioWrite(ORB_GPIO_BUZZER, OTA_BUZZER_ON_LEVEL);
    vTaskDelay(pdMS_TO_TICKS(100U));

    (void)nfwGpioWrite(ORB_GPIO_BUZZER, OTA_BUZZER_OFF_LEVEL);
    vTaskDelay(pdMS_TO_TICKS(100U));

    (void)nfwGpioWrite(ORB_GPIO_BUZZER, OTA_BUZZER_ON_LEVEL);
    vTaskDelay(pdMS_TO_TICKS(100U));

    (void)nfwGpioWrite(ORB_GPIO_BUZZER, OTA_BUZZER_OFF_LEVEL);
}

static void otaBuzzerValid(void)
{
    /* One long beep indicates the OTA image was accepted as valid. */

    (void)nfwGpioWrite(ORB_GPIO_BUZZER, OTA_BUZZER_ON_LEVEL);
    vTaskDelay(pdMS_TO_TICKS(300U));

    (void)nfwGpioWrite(ORB_GPIO_BUZZER, OTA_BUZZER_OFF_LEVEL);
}

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
    LoraTransportConfig_t loraConfig;

    float shuntResistanceOhm;
    uint16_t ina226Calibration;
    uint32_t hardwareFingerprint;

    const esp_partition_t *runningPartition;
    esp_ota_img_states_t otaState;
    bool otaPendingVerify = false;

    bool eolReady = false;

    printf("\n");
    printf("=====================================\n");
    printf(" Orb Drive Firmware\n");
    printf(" ESP32-S3\n");
    printf(" Board: %s\n", ORB_BOARD_NAME);
    printf(" Version: %s\n", ORB_BOARD_VERSION);
    printf("=====================================\n");

    /* =========================================================================
     * OTA buzzer initialization
     * ======================================================================= */

    otaBuzzerInit();

    /* =========================================================================
     * OTA rollback validation
     * ======================================================================= */

    runningPartition = esp_ota_get_running_partition();

    if (runningPartition != NULL &&
        esp_ota_get_state_partition(runningPartition, &otaState) == ESP_OK) {

        if (otaState == ESP_OTA_IMG_PENDING_VERIFY) {
            otaPendingVerify = true;

            printf("OTA image state: PENDING_VERIFY\n");
            printf("OTA rollback validation in progress.\n");

            otaBuzzerPending();
        }
    }

    /* =========================================================================
     * 1. Initialize EOL/NVS storage
     * ======================================================================= */

    status = eolStorageInit();

    if (status != NFW_STATUS_OK) {
        printf("EOL storage initialization FAILED: %d\n",
               (int)status);

        if (otaPendingVerify) {
            printf("OTA validation FAILED - rolling back.\n");
            esp_ota_mark_app_invalid_rollback_and_reboot();
        }

        return;
    }

    printf("EOL storage initialization: PASS\n");

    /* =========================================================================
     * 1A. Initialize Gateway-configurable OC configuration
     * ======================================================================= */

    status = ocConfigInit();

    if (status != NFW_STATUS_OK) {
        printf("OC configuration initialization FAILED: %d\n",
               (int)status);

        if (otaPendingVerify) {
            printf("OTA validation FAILED - rolling back.\n");
            esp_ota_mark_app_invalid_rollback_and_reboot();
        }

        return;
    }

    printf("OC configuration initialization: PASS\n");

    /* =========================================================================
     * 1B. Initialize Gateway-configurable voltage configuration
     * ======================================================================= */

    status = voltageConfigInit();

    if (status != NFW_STATUS_OK) {
        printf("Voltage configuration initialization FAILED: %d\n",
               (int)status);

        if (otaPendingVerify) {
            printf("OTA validation FAILED - rolling back.\n");
            esp_ota_mark_app_invalid_rollback_and_reboot();
        }

        return;
    }

    printf("Voltage configuration initialization: PASS\n");

    /* =========================================================================
     * 1C. Initialize SX1262 LoRa transport
     *
     * LoRa is the Node <-> Gateway transport layer.
     * RS-485 remains dedicated to the flow sensors.
     * ======================================================================= */

    loraConfig.spiHost = 2U;

    loraConfig.frequencyHz =
        LORA_TRANSPORT_DEFAULT_FREQUENCY_HZ;

    loraConfig.bandwidth =
        LORA_TRANSPORT_DEFAULT_BANDWIDTH;

    loraConfig.spreadingFactor =
        LORA_TRANSPORT_DEFAULT_SPREADING_FACTOR;

    loraConfig.codingRate =
        LORA_TRANSPORT_DEFAULT_CODING_RATE;

    loraConfig.txPowerDbm =
        LORA_TRANSPORT_DEFAULT_TX_POWER_DBM;

    status = loraTransportInit(&loraConfig);

    if (status != NFW_STATUS_OK) {
        printf("LoRa SX1262 initialization FAILED: %d\n",
               (int)status);

        if (otaPendingVerify) {
            printf("OTA validation FAILED - rolling back.\n");
            esp_ota_mark_app_invalid_rollback_and_reboot();
        }

        return;
    }

    printf("LoRa SX1262 initialization: PASS\n");
    printf("LoRa SPI host: SPI3\n");
    printf("LoRa frequency: %lu Hz\n",
           (unsigned long)loraConfig.frequencyHz);
    printf("LoRa spreading factor: %u\n",
           (unsigned)loraConfig.spreadingFactor);
    printf("LoRa TX power: %d dBm\n",
           (int)loraConfig.txPowerDbm);

    /* =========================================================================
     * 2. Determine current shunt configuration
     * ======================================================================= */

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

    /* =========================================================================
     * 3. Calculate and validate hardware/configuration fingerprint
     * ======================================================================= */

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

    /* =========================================================================
     * 4. Initialize I2C
     * ======================================================================= */

    i2cConfig.port = ORB_I2C_PORT;
    i2cConfig.sdaPin = ORB_I2C_SDA_GPIO;
    i2cConfig.sclPin = ORB_I2C_SCL_GPIO;
    i2cConfig.frequencyHz = ORB_I2C_FREQUENCY_HZ;

    status = nfwI2cInit(&i2cConfig);

    if (status != NFW_STATUS_OK) {
        printf("I2C initialization FAILED: %d\n",
               (int)status);

        if (otaPendingVerify) {
            printf("OTA validation FAILED - rolling back.\n");
            esp_ota_mark_app_invalid_rollback_and_reboot();
        }

        return;
    }

    printf("I2C initialization: PASS\n");
    printf("I2C SDA: GPIO%u\n",
           (unsigned)ORB_I2C_SDA_GPIO);
    printf("I2C SCL: GPIO%u\n",
           (unsigned)ORB_I2C_SCL_GPIO);
    printf("I2C frequency: %u Hz\n",
           (unsigned)ORB_I2C_FREQUENCY_HZ);

    /* =========================================================================
     * 5. Initialize INA226
     * ======================================================================= */

    inaConfig.i2cAddress =
        ORB_INA226_I2C_ADDRESS;

    inaConfig.shuntResistanceOhm =
        shuntResistanceOhm;

    inaConfig.maximumExpectedCurrentA =
        ORB_INA226_MAX_CURRENT_A;

    inaConfig.calibrationRegister =
        eolReady ? ina226Calibration : 0U;

    status = ina226Init(&inaConfig);

    if (status != NFW_STATUS_OK) {
        printf("INA226 initialization FAILED: %d\n",
               (int)status);

        if (otaPendingVerify) {
            printf("OTA validation FAILED - rolling back.\n");
            esp_ota_mark_app_invalid_rollback_and_reboot();
        }

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

    /* =========================================================================
     * 6. Initialize motor controller
     * ======================================================================= */

    status = motorControllerInit();

    if (status != NFW_STATUS_OK) {
        printf("Motor controller initialization FAILED: %d\n",
               (int)status);

        if (otaPendingVerify) {
            printf("OTA validation FAILED - rolling back.\n");
            esp_ota_mark_app_invalid_rollback_and_reboot();
        }

        return;
    }

    printf("Motor controller initialization: PASS\n");
    printf("Motor state: STOPPED\n");
    printf("Motor PWM: 0%%\n");

    /* =========================================================================
     * OTA image acceptance
     * ======================================================================= */

    if (otaPendingVerify) {
        if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) {
            printf("OTA rollback validation: PASS\n");
            printf("OTA image marked VALID.\n");

            otaBuzzerValid();
        } else {
            printf("OTA rollback validation: FAILED\n");
            printf("OTA image will be rolled back.\n");

            esp_ota_mark_app_invalid_rollback_and_reboot();
            return;
        }
    }
    /* =========================================================================
     * OTA Wi-Fi AP and HTTP server
     * ======================================================================= */

    status = otaWifiApInit();

    if (status != NFW_STATUS_OK) {
        printf("OTA Wi-Fi AP initialization FAILED: %d\n",
               (int)status);
        return;
    }

    printf("OTA Wi-Fi AP initialization: PASS\n");

    status = otaServerStart();

    if (status != NFW_STATUS_OK) {
        printf("OTA HTTP server initialization FAILED: %d\n",
               (int)status);

        (void)otaWifiApStop();
        return;
    }

    printf("OTA HTTP server initialization: PASS\n");

    /* =========================================================================
     * Final startup status
     * ======================================================================= */

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
