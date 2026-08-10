/**
 * @file nfw_uart.h
 * @brief Orb Drive UART Hardware Abstraction Layer.
 */

#ifndef NFW_UART_H
#define NFW_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "nfw_status.h"

/* ============================================================================
 * UART Configuration
 * ========================================================================== */

typedef struct
{
    uint32_t port;
    uint32_t baudRate;
    uint8_t dataBits;
    uint8_t stopBits;
    bool parityEnabled;
} NfwUartConfig_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

/**
 * @brief Initialize a UART peripheral.
 *
 * @param config UART configuration.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwUartInit(const NfwUartConfig_t *config);

/**
 * @brief Write bytes to UART.
 *
 * @param port UART peripheral number.
 * @param data Data buffer.
 * @param length Number of bytes to write.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwUartWrite(
    uint32_t port,
    const uint8_t *data,
    uint32_t length
);

/**
 * @brief Read bytes from UART.
 *
 * @param port UART peripheral number.
 * @param data Destination buffer.
 * @param length Maximum number of bytes to read.
 * @param bytesRead Number of bytes actually received.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwUartRead(
    uint32_t port,
    uint8_t *data,
    uint32_t length,
    uint32_t *bytesRead
);

/**
 * @brief Deinitialize a UART peripheral.
 *
 * @param port UART peripheral number.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwUartDeinit(uint32_t port);

#ifdef __cplusplus
}
#endif

#endif /* NFW_UART_H */
