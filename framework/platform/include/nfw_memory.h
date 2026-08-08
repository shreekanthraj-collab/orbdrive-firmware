/**
 * @file nfw_memory.h
 * @brief Platform memory abstraction.
 *
 * This module provides a platform-independent interface for dynamic
 * memory allocation used throughout the Orb Drive Firmware Framework.
 */

#ifndef NFW_MEMORY_H
#define NFW_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/*===========================================================================
 * Public API
 *===========================================================================*/

/**
 * @brief Allocates a memory block.
 *
 * @param size Number of bytes to allocate.
 *
 * @return Pointer to allocated memory or NULL on failure.
 */
void *nfwMalloc(size_t size);

/**
 * @brief Releases previously allocated memory.
 *
 * @param ptr Pointer returned by nfwMalloc().
 */
void nfwFree(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* NFW_MEMORY_H */
