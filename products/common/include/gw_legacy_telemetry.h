/**
 * @file gw_legacy_telemetry.h
 * @brief LoRa Node -> Municipal Gateway telemetry/event wire contract.
 *
 * This module matches the production Gateway's existing GW_PKT_STATUS and
 * GW_PKT_EVENT wire packets.
 */

#ifndef GW_LEGACY_TELEMETRY_H
#define GW_LEGACY_TELEMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "nfw_status.h"

#define GW_TELEMETRY_PACKET_TYPE      (0x20U)
#define GW_EVENT_PACKET_TYPE          (0x30U)

#define GW_TELEMETRY_CRC8_POLY        (0x31U)

#define GW_TELEMETRY_PACKET_LENGTH    (14U)
#define GW_EVENT_PACKET_LENGTH        (10U)

#define GW_TELEMETRY_FAULT_LOCK       (0x01U)
#define GW_TELEMETRY_FAULT_VOLTAGE    (0x02U)
#define GW_TELEMETRY_FAULT_I2C        (0x04U)
#define GW_TELEMETRY_FAULT_OC         (0x08U)
#define GW_TELEMETRY_FAULT_ACTUATOR   (0x10U)
#define GW_TELEMETRY_FAULT_DISENGAGE  (0x20U)

/*
 * Additive production events for explicit bypass-state telemetry.
 * Existing event IDs remain unchanged.
 */
#define GW_EVENT_BYPASS_ACTIVE        (0x0FU)
#define GW_EVENT_BYPASS_CANCELLED     (0x10U)

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

typedef struct
{
    uint8_t node;
    uint8_t sequence;
    uint8_t event;
    uint16_t voltage100;
    uint16_t current100;
    uint8_t gateway_id;
} GwLegacyEventSample_t;

uint8_t gwLegacyTelemetryCrc8(
    const uint8_t *data,
    uint32_t length);

NfwStatus_t gwLegacyTelemetryBuild(
    const GwLegacyTelemetrySample_t *sample,
    uint8_t *packet,
    uint32_t packetLength);

NfwStatus_t gwLegacyTelemetrySend(
    const GwLegacyTelemetrySample_t *sample,
    uint32_t timeoutMs);

NfwStatus_t gwLegacyEventBuild(
    const GwLegacyEventSample_t *sample,
    uint8_t *packet,
    uint32_t packetLength);

NfwStatus_t gwLegacyEventSend(
    const GwLegacyEventSample_t *sample,
    uint32_t timeoutMs);

#ifdef __cplusplus
}
#endif

#endif /* GW_LEGACY_TELEMETRY_H */
