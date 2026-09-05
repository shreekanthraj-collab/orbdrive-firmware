/**
 * @file ota_wifi_ap.c
 * @brief Node Wi-Fi Access Point for OTA module communication.
 */

#include "ota_wifi_ap.h"

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"

/* ============================================================================
 * OTA Wi-Fi AP
 * ========================================================================== */

NfwStatus_t otaWifiApInit(void)
{
    esp_err_t err;

    err = esp_netif_init();

    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return NFW_STATUS_ERROR;
    }

    err = esp_event_loop_create_default();

    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return NFW_STATUS_ERROR;
    }

    if (esp_netif_create_default_wifi_ap() == NULL) {
        return NFW_STATUS_ERROR;
    }

    wifi_init_config_t wifiInitConfig = WIFI_INIT_CONFIG_DEFAULT();

    err = esp_wifi_init(&wifiInitConfig);

    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return NFW_STATUS_ERROR;
    }

    wifi_config_t apConfig = {
        .ap = {
            .ssid = OTA_WIFI_AP_SSID,
            .ssid_len = sizeof(OTA_WIFI_AP_SSID) - 1U,
            .channel = OTA_WIFI_AP_CHANNEL,
            .password = OTA_WIFI_AP_PASSWORD,
            .max_connection = OTA_WIFI_AP_MAX_CLIENTS,
            .authmode = WIFI_AUTH_WPA2_PSK
        }
    };

    err = esp_wifi_set_mode(WIFI_MODE_AP);

    if (err != ESP_OK) {
        return NFW_STATUS_ERROR;
    }

    err = esp_wifi_set_config(WIFI_IF_AP, &apConfig);

    if (err != ESP_OK) {
        return NFW_STATUS_ERROR;
    }

    err = esp_wifi_start();

    if (err != ESP_OK) {
        return NFW_STATUS_ERROR;
    }

    return NFW_STATUS_OK;
}

NfwStatus_t otaWifiApStop(void)
{
    esp_err_t err;

    err = esp_wifi_stop();

    if (err != ESP_OK && err != ESP_ERR_WIFI_NOT_INIT) {
        return NFW_STATUS_ERROR;
    }

    return NFW_STATUS_OK;
}
