#ifndef OTA_SERVER_H
#define OTA_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nfw_status.h"

/**
 * @brief Start the HTTP OTA upload server.
 *
 * OTA module uploads firmware to the Node over the Node-created
 * Wi-Fi access point.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t otaServerStart(void);

/**
 * @brief Stop the HTTP OTA upload server.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t otaServerStop(void);

#ifdef __cplusplus
}
#endif

#endif /* OTA_SERVER_H */
