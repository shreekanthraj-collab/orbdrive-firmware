/**
 * @file nfw_platform.h
 * @brief Orb Drive Platform public interface.
 *
 * The Platform component provides a hardware-independent abstraction
 * for operating system and platform services required by the firmware.
 *
 * It exposes the platform initialization entry point and serves as the
 * root interface for time, memory and synchronization abstractions.
 */

#ifndef NFW_PLATFORM_H
#define NFW_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "nfw_types.h"
#include "nfw_status.h"

/*===========================================================================
 * Public API
 *===========================================================================*/

/**
 * @brief Initializes the Platform component.
 *
 * This function prepares all platform-level services required by the
 * framework. It shall be called once during system startup before any
 * higher-level framework component is initialized.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwPlatformInit(void);

/**
 * @brief Returns whether the Platform component is initialized.
 *
 * @return true if initialized.
 * @return false otherwise.
 */
bool nfwPlatformIsInitialized(void);

#ifdef __cplusplus
}
#endif

#endif /* NFW_PLATFORM_H */
