/**
 * @file nfw_core.h
 * @brief Orb Drive Framework Core public interface.
 */

#ifndef NFW_CORE_H
#define NFW_CORE_H

#include <stdbool.h>

#include "nfw_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Orb Drive Framework Core.
 *
 * This function must be called exactly once during system startup.
 *
 * @return
 *      - NFW_STATUS_OK                  Initialization successful.
 *      - NFW_STATUS_ALREADY_INITIALIZED Framework already initialized.
 */
NfwStatus_t nfwCoreInit(void);

/**
 * @brief Returns the initialization state of the framework.
 *
 * @return true  Framework initialized.
 * @return false Framework not initialized.
 */
bool nfwCoreIsInitialized(void);

#ifdef __cplusplus
}
#endif

#endif /* NFW_CORE_H */