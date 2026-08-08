/**
 * @file nfw_types.h
 * @brief Common fundamental types for the Orb Drive Firmware Framework.
 *
 * This header provides project-wide primitive definitions used across all
 * framework components. It intentionally contains no platform-specific code.
 */

#ifndef NFW_TYPES_H
#define NFW_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
 * Generic Handle
 *===========================================================================*/

typedef void* NfwHandle_t;

/*===========================================================================
 * Generic Callback
 *===========================================================================*/

typedef void (*NfwCallback_t)(void *context);

/*===========================================================================
 * Version Structure
 *===========================================================================*/

typedef struct
{
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
} NfwVersion_t;

#ifdef __cplusplus
}
#endif

#endif /* NFW_TYPES_H */
