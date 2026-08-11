#include "rs485.h"

#include <stddef.h>

#include "nfw_gpio.h"
#include "nfw_uart.h"

static uint32_t s_uart_port;
static uint32_t s_direction_pin;
static bool s_initialized;

NfwStatus_t rs485Init(const Rs485Config_t *config)
{
    if (config == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    NfwGpioConfig_t direction_config = {
        .pin = (uint32_t)config->direction_pin,
        .direction = NFW_GPIO_DIRECTION_OUTPUT,
        .pull = NFW_GPIO_PULL_NONE,
        .initialLevel = false
    };

    NfwStatus_t status = nfwGpioInit(&direction_config);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    NfwUartConfig_t uart_config = {
        .port = config->uart_port,
        .baudRate = config->baud_rate,
        .dataBits = config->data_bits,
        .stopBits = config->stop_bits,
        .parityEnabled = config->parity_enabled,
        .txPin = config->tx_pin,
        .rxPin = config->rx_pin
    };

    status = nfwUartInit(&uart_config);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    s_uart_port = config->uart_port;
    s_direction_pin = (uint32_t)config->direction_pin;
    s_initialized = true;

    return NFW_STATUS_OK;
}

NfwStatus_t rs485Transmit(
    const uint8_t *data,
    uint32_t length,
    uint32_t timeout_ms)
{
    if (!s_initialized)
    {
        return NFW_STATUS_ERROR;
    }

    if (data == NULL || length == 0U)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    NfwStatus_t status = nfwGpioWrite(
        s_direction_pin,
        true);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = nfwUartWrite(
        s_uart_port,
        data,
        length);

    if (status != NFW_STATUS_OK)
    {
        (void)nfwGpioWrite(s_direction_pin, false);
        return status;
    }

    status = nfwUartWaitTxDone(
        s_uart_port,
        timeout_ms);

    NfwStatus_t direction_status = nfwGpioWrite(
        s_direction_pin,
        false);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    return direction_status;
}

NfwStatus_t rs485Receive(
    uint8_t *data,
    uint32_t length,
    uint32_t *bytes_read)
{
    if (!s_initialized)
    {
        return NFW_STATUS_ERROR;
    }

    if (data == NULL || bytes_read == NULL || length == 0U)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    return nfwUartRead(
        s_uart_port,
        data,
        length,
        bytes_read);
}

NfwStatus_t rs485Deinit(void)
{
    if (!s_initialized)
    {
        return NFW_STATUS_OK;
    }

    NfwStatus_t status = nfwUartDeinit(s_uart_port);

    s_initialized = false;
    s_uart_port = 0U;
    s_direction_pin = 0U;

    return status;
}