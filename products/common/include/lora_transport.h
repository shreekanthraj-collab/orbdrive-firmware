/**
 * @file lora_transport.h
 * @brief Orb Drive SX1262 LoRa hardware transport interface.
 */

#ifndef LORA_TRANSPORT_H
#define LORA_TRANSPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "nfw_status.h"

/* ============================================================================
 * LoRa transport configuration
 * ========================================================================== */

typedef struct
{
    uint32_t spiHost;

    uint32_t frequencyHz;

    uint8_t bandwidth;
    uint8_t spreadingFactor;
    uint8_t codingRate;

    int8_t txPowerDbm;

} LoraTransportConfig_t;

/* ============================================================================
 * Default configuration
 *
 * These are transport defaults only.
 * Gateway/application configuration will be handled later.
 * ========================================================================== */

#define LORA_TRANSPORT_DEFAULT_FREQUENCY_HZ      (868000000UL)

/*
 * These symbolic values will be mapped to SX1262 register/command
 * representations by the implementation.
 */
#define LORA_TRANSPORT_DEFAULT_BANDWIDTH         (0U)
#define LORA_TRANSPORT_DEFAULT_SPREADING_FACTOR  (7U)
#define LORA_TRANSPORT_DEFAULT_CODING_RATE       (1U)
#define LORA_TRANSPORT_DEFAULT_TX_POWER_DBM      (17)

/* ============================================================================
 * Public API
 * ========================================================================== */

/**
 * @brief Initialize the SX1262 hardware transport.
 *
 * Initializes:
 * - SPI
 * - SX1262 CS/NSS
 * - SX1262 RESET
 * - SX1262 BUSY
 * - SX1262 DIO1
 * - SX1262 RXEN
 * - SX1262 TXEN
 *
 * @param config LoRa transport configuration.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t loraTransportInit(
    const LoraTransportConfig_t *config);

/**
 * @brief Check whether the SX1262 transport is initialized.
 *
 * @return true when initialized.
 */
bool loraTransportIsInitialized(void);

/**
 * @brief Wait until the SX1262 BUSY signal is released.
 *
 * @param timeoutMs Maximum wait time.
 *
 * @return NFW_STATUS_OK when BUSY is released.
 */
NfwStatus_t loraTransportWaitReady(
    uint32_t timeoutMs);

/**
 * @brief Hardware reset of the SX1262.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t loraTransportReset(void);

/**
 * @brief Transmit a raw LoRa payload.
 *
 * This is a transport-level API only.
 * Gateway packet formatting is implemented later.
 *
 * @param data Payload buffer.
 * @param length Payload length in bytes.
 * @param timeoutMs Maximum operation time.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t loraTransportTransmit(
    const uint8_t *data,
    uint32_t length,
    uint32_t timeoutMs);

/**
 * @brief Receive a raw LoRa payload.
 *
 * @param data Destination buffer.
 * @param maxLength Maximum payload length.
 * @param receivedLength Number of received bytes.
 * @param timeoutMs Maximum wait time.
 *
 * @return NFW_STATUS_OK on successful reception.
 */
NfwStatus_t loraTransportReceive(
    uint8_t *data,
    uint32_t maxLength,
    uint32_t *receivedLength,
    uint32_t timeoutMs);

/**
 * @brief Put the SX1262 into receive mode.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t loraTransportStartReceive(void);

/**
 * @brief Stop LoRa activity and place the radio in standby.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t loraTransportStandby(void);

/**
 * @brief Deinitialize the SX1262 transport.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t loraTransportDeinit(void);

#ifdef __cplusplus
}
#endif

#endif /* LORA_TRANSPORT_H */