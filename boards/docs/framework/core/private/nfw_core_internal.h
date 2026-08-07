/**
 * @file nfw_core_internal.h
 * @brief Internal Framework Core definitions.
 *
 * This header is private to the Framework Core component.
 * It must never be included by external components.
 */

#ifndef NFW_CORE_INTERNAL_H
#define NFW_CORE_INTERNAL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Internal Core context.
 */
typedef struct
{
    bool initialized;
} NfwCoreContext_t;

/**
 * @brief Returns the internal framework context.
 *
 * @return Pointer to the internal context.
 */
NfwCoreContext_t *nfwCoreContext(void);

#ifdef __cplusplus
}
#endif

#endif /* NFW_CORE_INTERNAL_H */
