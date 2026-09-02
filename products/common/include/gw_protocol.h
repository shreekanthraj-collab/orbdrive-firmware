/**
 * @file gw_protocol.h
 * @brief Orb Drive Node <-> Gateway protocol contract.
 */

#ifndef GW_PROTOCOL_H
#define GW_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "nfw_status.h"

/* ============================================================================
 * Protocol limits
 * ========================================================================== */

#define GW_PROTOCOL_VERSION              (1U)
#define GW_PROTOCOL_MAX_PAYLOAD_LENGTH   (64U)
#define GW_PROTOCOL_HEADER_LENGTH        (8U)
#define GW_PROTOCOL_CRC_LENGTH           (2U)
#define GW_PROTOCOL_MAX_PACKET_LENGTH    \
    (GW_PROTOCOL_HEADER_LENGTH + GW_PROTOCOL_MAX_PAYLOAD_LENGTH + GW_PROTOCOL_CRC_LENGTH)

/* ============================================================================
 * Packet types
 * ========================================================================== */

typedef enum
{
    GW_PACKET_TYPE_NONE = 0x00U,
    GW_PACKET_TYPE_COMMAND = 0x01U,
    GW_PACKET_TYPE_RESPONSE = 0x02U,
    GW_PACKET_TYPE_TELEMETRY = 0x03U

} GwPacketType_t;

/* ============================================================================
 * Gateway command identifiers
 * ========================================================================== */

typedef enum
{
    GW_COMMAND_NONE = 0x00U,

    GW_COMMAND_MOTOR_START = 0x01U,
    GW_COMMAND_MOTOR_STOP = 0x02U,
    GW_COMMAND_EMERGENCY_STOP = 0x03U,
    GW_COMMAND_CLEAR_EMERGENCY_STOP = 0x04U,

    GW_COMMAND_GET_TELEMETRY = 0x10U,

    GW_COMMAND_SET_OC_CONFIG = 0x20U,
    GW_COMMAND_SET_VOLTAGE_CONFIG = 0x21U

} GwCommandId_t;

/* ============================================================================
 * Command result
 * ========================================================================== */

typedef enum
{
    GW_COMMAND_RESULT_OK = 0x00U,
    GW_COMMAND_RESULT_ERROR = 0x01U,
    GW_COMMAND_RESULT_INVALID_COMMAND = 0x02U,
    GW_COMMAND_RESULT_INVALID_ARGUMENT = 0x03U,
    GW_COMMAND_RESULT_INVALID_STATE = 0x04U,
    GW_COMMAND_RESULT_NOT_SUPPORTED = 0x05U

} GwCommandResult_t;

/* ============================================================================
 * Packet header
 * ========================================================================== */

typedef struct
{
    uint8_t version;
    GwPacketType_t type;

    uint16_t nodeId;
    uint16_t sequence;

    uint16_t payloadLength;

} GwPacketHeader_t;

/* ============================================================================
 * Command packet
 * ========================================================================== */

typedef struct
{
    GwPacketHeader_t header;

    GwCommandId_t command;

    const uint8_t *payload;
    uint16_t payloadLength;

} GwCommandPacket_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

/**
 * @brief Initialize the gateway protocol subsystem.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t gwProtocolInit(void);

/**
 * @brief Build a gateway command packet.
 *
 * @param buffer Destination packet buffer.
 * @param bufferLength Destination buffer capacity.
 * @param nodeId Destination node identifier.
 * @param sequence Packet sequence number.
 * @param command Gateway command identifier.
 * @param payload Optional command payload.
 * @param payloadLength Payload length.
 * @param packetLength Actual encoded packet length.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t gwProtocolBuildCommand(
    uint8_t *buffer,
    uint16_t bufferLength,
    uint16_t nodeId,
    uint16_t sequence,
    GwCommandId_t command,
    const uint8_t *payload,
    uint16_t payloadLength,
    uint16_t *packetLength);

/**
 * @brief Parse a received gateway command packet.
 *
 * @param buffer Received packet.
 * @param packetLength Received packet length.
 * @param packet Destination command packet structure.
 *
 * @return NFW_STATUS_OK when the packet is valid.
 */
NfwStatus_t gwProtocolParseCommand(
    const uint8_t *buffer,
    uint16_t packetLength,
    GwCommandPacket_t *packet);

/**
 * @brief Validate a gateway protocol packet.
 *
 * Validates protocol version, packet type, payload length,
 * packet length, and CRC.
 *
 * @param buffer Packet buffer.
 * @param packetLength Packet length.
 *
 * @return NFW_STATUS_OK when valid.
 */
NfwStatus_t gwProtocolValidatePacket(
    const uint8_t *buffer,
    uint16_t packetLength);

/**
 * @brief Calculate protocol CRC-16.
 *
 * @param data Data buffer.
 * @param length Data length.
 *
 * @return Calculated CRC-16.
 */
uint16_t gwProtocolCalculateCrc(
    const uint8_t *data,
    uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* GW_PROTOCOL_H */
