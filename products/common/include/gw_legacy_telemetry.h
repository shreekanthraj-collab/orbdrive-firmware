/**
 * @file gw_legacy_telemetry.h
 * @brief LoRa Node -> Municipal Gateway telemetry wire contract.
 *
 * This module intentionally matches the production Gateway's existing
 * GW_PKT_STATUS (0x20) 14-byte packet. The frozen Gateway decoder is the
 * source of truth for this wire format.
 */

#ifndef GW_LEGACY_TELEMETRY_H
#define GW_LEGACY_TELEMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "nfw_status.h"

#define GW_TELEMETRY_PACKET_TYPE      (0x20U)
#define GW_TELEMETRY_CRC8_POLY        (0x31U)
#define GW_TELEMETRY_PACKET_LENGTH    (14U)

#define GW_TELEMETRY_FAULT_LOCK       (0x01U)
#define GW_TELEMETRY_FAULT_VOLTAGE    (0x02U)
#define GW_TELEMETRY_FAULT_I2C        (0x04U)
#define GW_TELEMETRY_FAULT_OC         (0x08U)
#define GW_TELEMETRY_FAULT_ACTUATOR   (0x10U)
#define GW_TELEMETRY_FAULT_DISENGAGE  (0x20U)

typedef struct
{
    uint8_t node;
    uint8_t sequence;
    uint8_t motor_state;
    uint8_t fault_flags;
    uint16_t turns100;
    uint16_t voltage100;
    uint16_t current100;
    int8_t rssi;
    uint8_t gateway_id;
} GwLegacyTelemetrySample_t;

/**
 * @brief Calculate the production Gateway CRC-8.
 */
uint8_t gwLegacyTelemetryCrc8(
    const uint8_t *data,
    uint32_t length);

/**
 * @brief Build the exact 14-byte status packet consumed by the Gateway.
 */
NfwStatus_t gwLegacyTelemetryBuild(
    const GwLegacyTelemetrySample_t *sample,
    uint8_t *packet,
    uint32_t packetLength);

/**
 * @brief Build and transmit one status packet through the LoRa transport.
 */
NfwStatus_t gwLegacyTelemetrySend(
    const GwLegacyTelemetrySample_t *sample,
    uint32_t timeoutMs);

#ifdef __cplusplus
}
#endif

#endif /* GW_LEGACY_TELEMETRY_H */
