#include "nfw_uart.h"

#include "driver/uart.h"

NfwStatus_t nfwUartInit(const NfwUartConfig_t *config)
{
    if (config == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->dataBits < 5U || config->dataBits > 8U)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->stopBits < 1U || config->stopBits > 2U)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    uart_word_length_t dataBits;

    switch (config->dataBits)
    {
        case 5U:
            dataBits = UART_DATA_5_BITS;
            break;

        case 6U:
            dataBits = UART_DATA_6_BITS;
            break;

        case 7U:
            dataBits = UART_DATA_7_BITS;
            break;

        case 8U:
            dataBits = UART_DATA_8_BITS;
            break;

        default:
            return NFW_STATUS_INVALID_ARGUMENT;
    }

    uart_stop_bits_t stopBits;

    switch (config->stopBits)
    {
        case 1U:
            stopBits = UART_STOP_BITS_1;
            break;

        case 2U:
            stopBits = UART_STOP_BITS_2;
            break;

        default:
            return NFW_STATUS_INVALID_ARGUMENT;
    }

    uart_config_t uartConfig = {
        .baud_rate = (int)config->baudRate,
        .data_bits = dataBits,
        .parity = config->parityEnabled
                      ? UART_PARITY_EVEN
                      : UART_PARITY_DISABLE,
        .stop_bits = stopBits,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    esp_err_t err = uart_param_config(
        (uart_port_t)config->port,
        &uartConfig);

    if (err != ESP_OK)
    {
        return NFW_STATUS_ERROR;
    }

    /*
     * GPIO pin assignment is intentionally not performed here.
     * Physical TX/RX routing belongs to the board definition layer.
     */
    err = uart_set_pin(
        (uart_port_t)config->port,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE);

    if (err != ESP_OK)
    {
        return NFW_STATUS_ERROR;
    }

    err = uart_driver_install(
        (uart_port_t)config->port,
        1024U,
        1024U,
        0U,
        NULL,
        0U);

    if (err != ESP_OK)
    {
        return NFW_STATUS_ERROR;
    }

    return NFW_STATUS_OK;
}

NfwStatus_t nfwUartWrite(
    uint32_t port,
    const uint8_t *data,
    uint32_t length)
{
    if (data == NULL || length == 0U)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    int written = uart_write_bytes(
        (uart_port_t)port,
        data,
        length);

    if (written < 0 || (uint32_t)written != length)
    {
        return NFW_STATUS_ERROR;
    }

    return NFW_STATUS_OK;
}

NfwStatus_t nfwUartRead(
    uint32_t port,
    uint8_t *data,
    uint32_t length,
    uint32_t *bytesRead)
{
    if (data == NULL || bytesRead == NULL || length == 0U)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    int received = uart_read_bytes(
        (uart_port_t)port,
        data,
        length,
        0);

    if (received < 0)
    {
        *bytesRead = 0U;
        return NFW_STATUS_ERROR;
    }

    *bytesRead = (uint32_t)received;

    return NFW_STATUS_OK;
}

NfwStatus_t nfwUartDeinit(uint32_t port)
{
    esp_err_t err = uart_driver_delete(
        (uart_port_t)port);

    if (err != ESP_OK)
    {
        return NFW_STATUS_ERROR;
    }

    return NFW_STATUS_OK;
}