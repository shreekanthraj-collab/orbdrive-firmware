/**
 * @file nfw_version.h
 * @brief Orb Drive Framework version interface.
 */

#ifndef NFW_VERSION_H
#define NFW_VERSION_H

#include "nfw_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
 * Framework Version
 *===========================================================================*/

#define NFW_VERSION_MAJOR      0
#define NFW_VERSION_MINOR      1
#define NFW_VERSION_PATCH      0

#define NFW_VERSION_STRING     "0.1.0"

/*===========================================================================
 * Public API
 *===========================================================================*/

/**
 * @brief Returns the framework version structure.
 *
 * @return Pointer to immutable framework version.
 */
const NfwVersion_t *nfwVersionGet(void);

/**
 * @brief Returns the framework version string.
 *
 * @return Version string.
 */
const char *nfwVersionString(void);

#ifdef __cplusplus
}
#endif

#endif /* NFW_VERSION_H */