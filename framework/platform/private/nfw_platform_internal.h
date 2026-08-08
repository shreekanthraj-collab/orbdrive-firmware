/**
 * @file nfw_platform_internal.h
 * @brief Internal Platform definitions.
 *
 * This header is private to the Platform component and must never be
 * included outside this component.
 */

#ifndef NFW_PLATFORM_INTERNAL_H
#define NFW_PLATFORM_INTERNAL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
 * Internal Platform Context
 *===========================================================================*/

typedef struct
{
    bool initialized;
} NfwPlatformContext_t;

/*===========================================================================
 * Internal Functions
 *===========================================================================*/

/**
 * @brief Returns the internal Platform context.
 *
 * @return Pointer to the Platform context.
 */
NfwPlatformContext_t *nfwPlatformContext(void);

#ifdef __cplusplus
}
#endif

#endif /* NFW_PLATFORM_INTERNAL_H */
