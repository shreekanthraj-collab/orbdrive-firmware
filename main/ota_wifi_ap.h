#ifndef OTA_WIFI_AP_H
#define OTA_WIFI_AP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nfw_status.h"

/* ============================================================================
 * OTA Wi-Fi Access Point
 *
 * Node creates the AP.
 * OTA module joins this network.
 * ========================================================================== */

#define OTA_WIFI_AP_SSID        "lora-node"
#define OTA_WIFI_AP_PASSWORD    "node@1234"

#define OTA_WIFI_AP_CHANNEL     (1)
#define OTA_WIFI_AP_MAX_CLIENTS (1)

/**
 * @brief Initialize the Node OTA Wi-Fi access point.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t otaWifiApInit(void);

/**
 * @brief Stop the Node OTA Wi-Fi access point.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t otaWifiApStop(void);

#ifdef __cplusplus
}
#endif

#endif /* OTA_WIFI_AP_H */
