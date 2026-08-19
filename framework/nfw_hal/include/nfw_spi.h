/**
 * @file nfw_spi.h
 * @brief Orb Drive SPI Hardware Abstraction Layer.
 */

#ifndef NFW_SPI_H
#define NFW_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "nfw_status.h"

/* ============================================================================
 * SPI configuration
 * ========================================================================== */

typedef struct
{
    uint32_t host;
    uint32_t clockHz;

    int32_t mosiPin;
    int32_t misoPin;
    int32_t sclkPin;
    int32_t csPin;

} NfwSpiConfig_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

/**
 * @brief Initialize an SPI bus and register its chip-select device.
 *
 * @param config SPI configuration.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwSpiInit(
    const NfwSpiConfig_t *config);

/**
 * @brief Perform a full-duplex SPI transaction.
 *
 * @param host SPI host.
 * @param txData Transmit buffer.
 * @param rxData Receive buffer.
 * @param length Number of bytes to transfer.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwSpiTransfer(
    uint32_t host,
    const uint8_t *txData,
    uint8_t *rxData,
    uint32_t length);

/**
 * @brief Deinitialize an SPI bus.
 *
 * @param host SPI host.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwSpiDeinit(
    uint32_t host);

/**
 * @brief Check whether an SPI bus is initialized.
 *
 * @param host SPI host.
 *
 * @return true when initialized.
 */
bool nfwSpiIsInitialized(
    uint32_t host);

#ifdef __cplusplus
}
#endif

#endif /* NFW_SPI_H */