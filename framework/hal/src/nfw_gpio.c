#include "nfw_gpio.h"
#include "nfw_hal_internal.h"

NfwStatus_t nfwGpioInit(const NfwGpioConfig_t *config)
{
    if (config == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    return NFW_STATUS_OK;
}

NfwStatus_t nfwGpioWrite(uint32_t pin, bool level)
{
    (void)pin;
    (void)level;

    return NFW_STATUS_OK;
}

NfwStatus_t nfwGpioRead(uint32_t pin, bool *level)
{
    (void)pin;

    if (level == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    *level = false;

    return NFW_STATUS_OK;
}

NfwStatus_t nfwGpioToggle(uint32_t pin)
{
    (void)pin;

    return NFW_STATUS_OK;
}