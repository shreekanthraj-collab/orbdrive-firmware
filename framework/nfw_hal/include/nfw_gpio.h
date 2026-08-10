/**
 * @file nfw_gpio.h
 * @brief Orb Drive GPIO Hardware Abstraction Layer.
 */

#ifndef NFW_GPIO_H
#define NFW_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "nfw_status.h"

/* ============================================================================
 * GPIO Direction
 * ========================================================================== */

typedef enum
{
    NFW_GPIO_DIRECTION_INPUT = 0,
    NFW_GPIO_DIRECTION_OUTPUT
} NfwGpioDirection_t;

/* ============================================================================
 * GPIO Pull Configuration
 * ========================================================================== */

typedef enum
{
    NFW_GPIO_PULL_NONE = 0,
    NFW_GPIO_PULL_UP,
    NFW_GPIO_PULL_DOWN
} NfwGpioPull_t;

/* ============================================================================
 * GPIO Configuration
 * ========================================================================== */

typedef struct
{
    uint32_t pin;
    NfwGpioDirection_t direction;
    NfwGpioPull_t pull;
    bool initialLevel;
} NfwGpioConfig_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

/**
 * @brief Initialize a GPIO.
 *
 * @param config GPIO configuration.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwGpioInit(const NfwGpioConfig_t *config);

/**
 * @brief Set GPIO output level.
 *
 * @param pin GPIO number.
 * @param level true for HIGH, false for LOW.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwGpioWrite(uint32_t pin, bool level);

/**
 * @brief Read GPIO input level.
 *
 * @param pin GPIO number.
 * @param level Pointer receiving the GPIO level.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwGpioRead(uint32_t pin, bool *level);

/**
 * @brief Toggle GPIO output level.
 *
 * @param pin GPIO number.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwGpioToggle(uint32_t pin);

#ifdef __cplusplus
}
#endif

#endif /* NFW_GPIO_H */