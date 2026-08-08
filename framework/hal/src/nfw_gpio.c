/**
 * @file nfw_gpio.c
 * @brief ESP-IDF implementation of the Orb Drive GPIO HAL.
 */

#include "nfw_gpio.h"

#include "driver/gpio.h"

/* ============================================================================
 * Private Helpers
 * ========================================================================== */

static bool nfwGpioIsValidPin(uint32_t pin)
{
    return pin < GPIO_NUM_MAX;
}

static NfwStatus_t nfwGpioMapError(esp_err_t err)
{
    if (err == ESP_OK)
    {
        return NFW_STATUS_OK;
    }

    if (err == ESP_ERR_INVALID_ARG)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    return NFW_STATUS_ERROR;
}

/* ============================================================================
 * Public API
 * ========================================================================== */

NfwStatus_t nfwGpioInit(const NfwGpioConfig_t *config)
{
    if (config == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!nfwGpioIsValidPin(config->pin))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    gpio_config_t gpioConfig = {
        .pin_bit_mask = (1ULL << config->pin),

        .mode = (config->direction == NFW_GPIO_DIRECTION_OUTPUT)
                    ? GPIO_MODE_OUTPUT
                    : GPIO_MODE_INPUT,

        .pull_up_en = (config->pull == NFW_GPIO_PULL_UP)
                          ? GPIO_PULLUP_ENABLE
                          : GPIO_PULLUP_DISABLE,

        .pull_down_en = (config->pull == NFW_GPIO_PULL_DOWN)
                            ? GPIO_PULLDOWN_ENABLE
                            : GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE
    };

    NfwStatus_t status = nfwGpioMapError(gpio_config(&gpioConfig));

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    if (config->direction == NFW_GPIO_DIRECTION_OUTPUT)
    {
        status = nfwGpioMapError(
            gpio_set_level(
                (gpio_num_t)config->pin,
                config->initialLevel ? 1 : 0
            )
        );
    }

    return status;
}

NfwStatus_t nfwGpioWrite(uint32_t pin, bool level)
{
    if (!nfwGpioIsValidPin(pin))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    return nfwGpioMapError(
        gpio_set_level(
            (gpio_num_t)pin,
            level ? 1 : 0
        )
    );
}

NfwStatus_t nfwGpioRead(uint32_t pin, bool *level)
{
    if (level == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!nfwGpioIsValidPin(pin))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    *level = (gpio_get_level((gpio_num_t)pin) != 0);

    return NFW_STATUS_OK;
}

NfwStatus_t nfwGpioToggle(uint32_t pin)
{
    if (!nfwGpioIsValidPin(pin))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    int currentLevel = gpio_get_level((gpio_num_t)pin);

    if (currentLevel < 0)
    {
        return NFW_STATUS_ERROR;
    }

    return nfwGpioMapError(
        gpio_set_level(
            (gpio_num_t)pin,
            currentLevel ? 0 : 1
        )
    );
}