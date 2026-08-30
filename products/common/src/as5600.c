#include "as5600.h"

#include <stddef.h>

#include "nfw_i2c.h"

static bool s_initialized = false;
static uint8_t s_i2c_address = AS5600_I2C_ADDRESS;

NfwStatus_t as5600Init(
    const As5600Config_t *config)
{
    if (s_initialized)
    {
        return NFW_STATUS_ALREADY_INITIALIZED;
    }

    if (config == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    s_i2c_address = config->i2cAddress;

    if (!nfwI2cDevicePresent(s_i2c_address))
    {
        return NFW_STATUS_ERROR;
    }

    s_initialized = true;

    return NFW_STATUS_OK;
}

NfwStatus_t as5600ReadRawAngle(
    uint16_t *rawAngle)
{
    uint8_t reg = AS5600_REG_RAW_ANGLE;
    uint8_t data[2];
    NfwStatus_t status;

    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    if (rawAngle == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    status = nfwI2cWriteRead(
        s_i2c_address,
        &reg,
        1U,
        data,
        2U);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    *rawAngle =
        (uint16_t)(((uint16_t)data[0] << 8U) |
                   (uint16_t)data[1]);

    *rawAngle &= 0x0FFFU;

    return NFW_STATUS_OK;
}

NfwStatus_t as5600ReadAngleDegrees(
    float *angleDegrees)
{
    uint16_t rawAngle;
    NfwStatus_t status;

    if (angleDegrees == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    status = as5600ReadRawAngle(&rawAngle);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    *angleDegrees =
        ((float)rawAngle * 360.0f) / 4096.0f;

    return NFW_STATUS_OK;
}

bool as5600IsInitialized(void)
{
    return s_initialized;
}
