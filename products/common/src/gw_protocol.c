/**
 * @file gw_protocol.c
 * @brief Orb Drive Node <-> Gateway protocol implementation.
 */

#include "gw_protocol.h"

#include <stddef.h>

/* ============================================================================
 * Internal helpers
 * ========================================================================== */

static bool gwProtocolIsValidPacketType(
    GwPacketType_t type)
{
    return (type == GW_PACKET_TYPE_COMMAND) ||
           (type == GW_PACKET_TYPE_RESPONSE) ||
           (type == GW_PACKET_TYPE_TELEMETRY);
}

static bool gwProtocolIsValidCommand(
    GwCommandId_t command)
{
    return (command == GW_COMMAND_MOTOR_START) ||
           (command == GW_COMMAND_MOTOR_STOP) ||
           (command == GW_COMMAND_EMERGENCY_STOP) ||
           (command == GW_COMMAND_CLEAR_EMERGENCY_STOP) ||
           (command == GW_COMMAND_GET_TELEMETRY) ||
           (command == GW_COMMAND_SET_OC_CONFIG) ||
           (command == GW_COMMAND_SET_VOLTAGE_CONFIG) ||
           (command == GW_COMMAND_VOLTAGE_BYPASS) ||
           (command == GW_COMMAND_VOLTAGE_CANCEL);
}

static void gwProtocolWriteU16(
    uint8_t *buffer,
    uint16_t value)
{
    buffer[0] = (uint8_t)((value >> 8U) & 0xFFU);
    buffer[1] = (uint8_t)(value & 0xFFU);
}

static uint16_t gwProtocolReadU16(
    const uint8_t *buffer)
{
    return (uint16_t)(
        ((uint16_t)buffer[0] << 8U) |
        (uint16_t)buffer[1]);
}

/* ============================================================================
 * Initialization
 * ========================================================================== */

NfwStatus_t gwProtocolInit(void)
{
    return NFW_STATUS_OK;
}

/* ============================================================================
 * CRC-16
 * ========================================================================== */

uint16_t gwProtocolCalculateCrc(
    const uint8_t *data,
    uint16_t length)
{
    uint16_t crc = 0xFFFFU;
    uint16_t index;
    uint8_t bit;

    if ((data == NULL) && (length != 0U))
    {
        return 0U;
    }

    for (index = 0U; index < length; index++)
    {
        crc ^= data[index];

        for (bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x0001U) != 0U)
            {
                crc >>= 1U;
                crc ^= 0xA001U;
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return crc;
}

/* ============================================================================
 * Packet validation
 * ========================================================================== */

NfwStatus_t gwProtocolValidatePacket(
    const uint8_t *buffer,
    uint16_t packetLength)
{
    uint16_t payloadLength;
    uint16_t expectedLength;
    uint16_t receivedCrc;
    uint16_t calculatedCrc;

    if (buffer == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if ((packetLength < GW_PROTOCOL_HEADER_LENGTH + GW_PROTOCOL_CRC_LENGTH) ||
        (packetLength > GW_PROTOCOL_MAX_PACKET_LENGTH))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (buffer[0] != GW_PROTOCOL_VERSION)
    {
        return NFW_STATUS_ERROR;
    }

    if (!gwProtocolIsValidPacketType(
            (GwPacketType_t)buffer[1]))
    {
        return NFW_STATUS_ERROR;
    }

    payloadLength = gwProtocolReadU16(
        &buffer[6]);

    if (payloadLength > GW_PROTOCOL_MAX_PAYLOAD_LENGTH)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    expectedLength =
        (uint16_t)(
            GW_PROTOCOL_HEADER_LENGTH +
            payloadLength +
            GW_PROTOCOL_CRC_LENGTH);

    if (packetLength != expectedLength)
    {
        return NFW_STATUS_ERROR;
    }

    receivedCrc =
        gwProtocolReadU16(
            &buffer[packetLength - GW_PROTOCOL_CRC_LENGTH]);

    calculatedCrc =
        gwProtocolCalculateCrc(
            buffer,
            (uint16_t)(
                packetLength - GW_PROTOCOL_CRC_LENGTH));

    if (receivedCrc != calculatedCrc)
    {
        return NFW_STATUS_ERROR;
    }

    return NFW_STATUS_OK;
}

/* ============================================================================
 * Command builder
 * ========================================================================== */

NfwStatus_t gwProtocolBuildCommand(
    uint8_t *buffer,
    uint16_t bufferLength,
    uint16_t nodeId,
    uint16_t sequence,
    GwCommandId_t command,
    const uint8_t *payload,
    uint16_t payloadLength,
    uint16_t *packetLength)
{
    uint16_t totalLength;
    uint16_t crc;

    if ((buffer == NULL) ||
        (packetLength == NULL))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!gwProtocolIsValidCommand(command))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (payloadLength > GW_PROTOCOL_MAX_PAYLOAD_LENGTH)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if ((payloadLength != 0U) &&
        (payload == NULL))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    totalLength =
        (uint16_t)(
            GW_PROTOCOL_HEADER_LENGTH +
            1U +
            payloadLength +
            GW_PROTOCOL_CRC_LENGTH);

    if (bufferLength < totalLength)
    {
        return NFW_STATUS_NO_MEMORY;
    }

    /*
     * Header:
     *
     * Byte 0 : protocol version
     * Byte 1 : packet type
     * Byte 2 : node ID MSB
     * Byte 3 : node ID LSB
     * Byte 4 : sequence MSB
     * Byte 5 : sequence LSB
     * Byte 6 : payload length MSB
     * Byte 7 : payload length LSB
     *
     * Payload:
     *
     * Byte 8 : command ID
     * Byte 9+ : command-specific payload
     *
     * Final two bytes:
     *
     * CRC-16 MSB
     * CRC-16 LSB
     */

    buffer[0] = GW_PROTOCOL_VERSION;
    buffer[1] = GW_PACKET_TYPE_COMMAND;

    gwProtocolWriteU16(
        &buffer[2],
        nodeId);

    gwProtocolWriteU16(
        &buffer[4],
        sequence);

    /*
     * Payload length includes the command identifier.
     */
    gwProtocolWriteU16(
        &buffer[6],
        (uint16_t)(payloadLength + 1U));

    buffer[8] = (uint8_t)command;

    if (payloadLength != 0U)
    {
        for (uint16_t index = 0U;
             index < payloadLength;
             index++)
        {
            buffer[9U + index] = payload[index];
        }
    }

    crc =
        gwProtocolCalculateCrc(
            buffer,
            (uint16_t)(totalLength - GW_PROTOCOL_CRC_LENGTH));

    gwProtocolWriteU16(
        &buffer[totalLength - GW_PROTOCOL_CRC_LENGTH],
        crc);

    *packetLength = totalLength;

    return NFW_STATUS_OK;
}

/* ============================================================================
 * Command parser
 * ========================================================================== */

NfwStatus_t gwProtocolParseCommand(
    const uint8_t *buffer,
    uint16_t packetLength,
    GwCommandPacket_t *packet)
{
    uint16_t payloadLength;

    if ((buffer == NULL) ||
        (packet == NULL))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (gwProtocolValidatePacket(
            buffer,
            packetLength) != NFW_STATUS_OK)
    {
        return NFW_STATUS_ERROR;
    }

    if ((GwPacketType_t)buffer[1] !=
        GW_PACKET_TYPE_COMMAND)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    payloadLength =
        gwProtocolReadU16(
            &buffer[6]);

    if (payloadLength < 1U)
    {
        return NFW_STATUS_ERROR;
    }

    if (!gwProtocolIsValidCommand(
            (GwCommandId_t)buffer[8]))
    {
        return NFW_STATUS_ERROR;
    }

    packet->header.version =
        buffer[0];

    packet->header.type =
        (GwPacketType_t)buffer[1];

    packet->header.nodeId =
        gwProtocolReadU16(
            &buffer[2]);

    packet->header.sequence =
        gwProtocolReadU16(
            &buffer[4]);

    packet->header.payloadLength =
        payloadLength;

    packet->command =
        (GwCommandId_t)buffer[8];

    packet->payload =
        (payloadLength > 1U) ?
        &buffer[9] :
        NULL;

    packet->payloadLength =
        (uint16_t)(payloadLength - 1U);

    return NFW_STATUS_OK;
}
