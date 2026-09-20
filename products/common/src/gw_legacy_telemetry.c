/**
 * @file gw_legacy_telemetry.c
 * @brief LoRa Node -> Municipal Gateway telemetry/event implementation.
 */

#include "gw_legacy_telemetry.h"

#include <stddef.h>

#include "lora_transport.h"

static void putU16(
    uint8_t *buffer,
    uint16_t value)
{
    buffer[0] = (uint8_t)(value & 0xFFU);
    buffer[1] = (uint8_t)((value >> 8U) & 0xFFU);
}

uint8_t gwLegacyTelemetryCrc8(
    const uint8_t *data,
    uint32_t length)
{
    uint8_t crc = 0U;

    if (data == NULL)
    {
        return 0U;
    }

    for (uint32_t index = 0U; index < length; ++index)
    {
        crc ^= data[index];

        for (uint8_t bit = 0U; bit < 8U; ++bit)
        {
            if ((crc & 0x80U) != 0U)
            {
                crc = (uint8_t)((crc << 1U) ^
                                GW_TELEMETRY_CRC8_POLY);
            }
            else
            {
                crc <<= 1U;
            }
        }
    }

    return crc;
}

NfwStatus_t gwLegacyTelemetryBuild(
    const GwLegacyTelemetrySample_t *sample,
    uint8_t *packet,
    uint32_t packetLength)
{
    if (sample == NULL || packet == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (packetLength < GW_TELEMETRY_PACKET_LENGTH)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    packet[0] = sample->node;
    packet[1] = GW_TELEMETRY_PACKET_TYPE;
    packet[2] = sample->sequence;
    packet[3] = sample->motor_state;
    packet[4] = sample->fault_flags;

    putU16(&packet[5], sample->turns100);
    putU16(&packet[7], sample->voltage100);
    putU16(&packet[9], sample->current100);

    packet[11] = (uint8_t)sample->rssi;
    packet[12] = sample->gateway_id;
    packet[13] = gwLegacyTelemetryCrc8(packet, 13U);

    return NFW_STATUS_OK;
}

NfwStatus_t gwLegacyTelemetrySend(
    const GwLegacyTelemetrySample_t *sample,
    uint32_t timeoutMs)
{
    uint8_t packet[GW_TELEMETRY_PACKET_LENGTH];
    NfwStatus_t status;

    status = gwLegacyTelemetryBuild(
        sample,
        packet,
        sizeof(packet));

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    return loraTransportTransmit(
        packet,
        sizeof(packet),
        timeoutMs);
}

NfwStatus_t gwLegacyEventBuild(
    const GwLegacyEventSample_t *sample,
    uint8_t *packet,
    uint32_t packetLength)
{
    if (sample == NULL || packet == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (packetLength < GW_EVENT_PACKET_LENGTH)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    packet[0] = sample->node;
    packet[1] = GW_EVENT_PACKET_TYPE;
    packet[2] = sample->sequence;
    packet[3] = sample->event;

    putU16(&packet[4], sample->voltage100);
    putU16(&packet[6], sample->current100);

    packet[8] = sample->gateway_id;
    packet[9] = gwLegacyTelemetryCrc8(packet, 9U);

    return NFW_STATUS_OK;
}

NfwStatus_t gwLegacyEventSend(
    const GwLegacyEventSample_t *sample,
    uint32_t timeoutMs)
{
    uint8_t packet[GW_EVENT_PACKET_LENGTH];
    NfwStatus_t status;

    status = gwLegacyEventBuild(
        sample,
        packet,
        sizeof(packet));

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    return loraTransportTransmit(
        packet,
        sizeof(packet),
        timeoutMs);
}
