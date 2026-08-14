/**
 * @file nfw_pwm.c
 * @brief ESP-IDF implementation of the Orb Drive PWM HAL.
 */
#include <stdbool.h>
#include "nfw_pwm.h"

#include "driver/ledc.h"

#define NFW_PWM_MODE       LEDC_LOW_SPEED_MODE
#define NFW_PWM_TIMER      LEDC_TIMER_0
#define NFW_PWM_CHANNEL    LEDC_CHANNEL_0

static bool s_initialized = false;
static uint32_t s_pin = 0U;
static uint32_t s_frequency_hz = 0U;
static uint32_t s_resolution_bits = 0U;
static uint32_t s_duty_percent = 0U;

NfwStatus_t nfwPwmInit(
    uint32_t pin,
    uint32_t frequency_hz,
    uint32_t resolution_bits)
{
    if (frequency_hz == 0U)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (resolution_bits == 0U || resolution_bits > 14U)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (s_initialized)
    {
        return NFW_STATUS_ALREADY_INITIALIZED;
    }

    ledc_timer_config_t timer_config = {
        .speed_mode = NFW_PWM_MODE,
        .duty_resolution = (ledc_timer_bit_t)resolution_bits,
        .timer_num = NFW_PWM_TIMER,
        .freq_hz = frequency_hz,
        .clk_cfg = LEDC_AUTO_CLK
    };

    esp_err_t err = ledc_timer_config(&timer_config);

    if (err != ESP_OK)
    {
        return NFW_STATUS_ERROR;
    }

    ledc_channel_config_t channel_config = {
        .gpio_num = (int)pin,
        .speed_mode = NFW_PWM_MODE,
        .channel = NFW_PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = NFW_PWM_TIMER,
        .duty = 0U,
        .hpoint = 0U
    };

    err = ledc_channel_config(&channel_config);

    if (err != ESP_OK)
    {
        return NFW_STATUS_ERROR;
    }

    s_pin = pin;
    s_frequency_hz = frequency_hz;
    s_resolution_bits = resolution_bits;
    s_duty_percent = 0U;
    s_initialized = true;

    return NFW_STATUS_OK;
}

NfwStatus_t nfwPwmSetDutyPercent(uint32_t duty_percent)
{
    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    if (duty_percent > 100U)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    uint32_t max_duty = (1UL << s_resolution_bits) - 1UL;

    uint32_t duty =
        (duty_percent * max_duty + 50U) / 100U;

    esp_err_t err = ledc_set_duty(
        NFW_PWM_MODE,
        NFW_PWM_CHANNEL,
        duty);

    if (err != ESP_OK)
    {
        return NFW_STATUS_ERROR;
    }

    err = ledc_update_duty(
        NFW_PWM_MODE,
        NFW_PWM_CHANNEL);

    if (err != ESP_OK)
    {
        return NFW_STATUS_ERROR;
    }

    s_duty_percent = duty_percent;

    return NFW_STATUS_OK;
}

NfwStatus_t nfwPwmGetDutyPercent(uint32_t *duty_percent)
{
    if (duty_percent == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    *duty_percent = s_duty_percent;

    return NFW_STATUS_OK;
}

NfwStatus_t nfwPwmDisable(void)
{
    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    esp_err_t err = ledc_stop(
        NFW_PWM_MODE,
        NFW_PWM_CHANNEL,
        0);

    if (err != ESP_OK)
    {
        return NFW_STATUS_ERROR;
    }

    s_duty_percent = 0U;

    return NFW_STATUS_OK;
}