#include "valve_position.h"

#include <stddef.h>

#include "as5600.h"

static bool s_initialized = false;
static ValvePositionConfig_t s_config;

NfwStatus_t valvePositionInit(
    const ValvePositionConfig_t *config)
{
    if (s_initialized)
    {
        return NFW_STATUS_ALREADY_INITIALIZED;
    }

    if (config == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->closedAngleRaw > 4095U ||
        config->openAngleRaw > 4095U ||
        config->closedAngleRaw == config->openAngleRaw)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    s_config = *config;
    s_initialized = true;

    return NFW_STATUS_OK;
}

NfwStatus_t valvePositionGetRawAngle(
    uint16_t *rawAngle)
{
    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    return as5600ReadRawAngle(rawAngle);
}

NfwStatus_t valvePositionGetOpeningPercent(
    uint8_t *openingPercent)
{
    uint16_t rawAngle;
    uint16_t closedAngle;
    uint16_t openAngle;
    uint32_t numerator;
    uint32_t denominator;
    NfwStatus_t status;

    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    if (openingPercent == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    status = as5600ReadRawAngle(&rawAngle);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    closedAngle = s_config.closedAngleRaw;
    openAngle = s_config.openAngleRaw;

    if (openAngle > closedAngle)
    {
        if (rawAngle <= closedAngle)
        {
            *openingPercent = 0U;
            return NFW_STATUS_OK;
        }

        if (rawAngle >= openAngle)
        {
            *openingPercent = 100U;
            return NFW_STATUS_OK;
        }

        numerator = (uint32_t)(rawAngle - closedAngle);
        denominator = (uint32_t)(openAngle - closedAngle);
    }
    else
    {
        if (rawAngle >= closedAngle)
        {
            *openingPercent = 0U;
            return NFW_STATUS_OK;
        }

        if (rawAngle <= openAngle)
        {
            *openingPercent = 100U;
            return NFW_STATUS_OK;
        }

        numerator = (uint32_t)(closedAngle - rawAngle);
        denominator = (uint32_t)(closedAngle - openAngle);
    }

    *openingPercent =
        (uint8_t)((numerator * 100U + (denominator / 2U)) /
                  denominator);

    return NFW_STATUS_OK;
}
