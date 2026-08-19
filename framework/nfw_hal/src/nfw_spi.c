/**
 * @file nfw_spi.c
 * @brief ESP-IDF implementation of the Orb Drive SPI HAL.
 */

#include "nfw_spi.h"

#include <stddef.h>

#include "driver/spi_master.h"
#include "esp_err.h"

/* ============================================================================
 * Internal state
 * ========================================================================== */

#define NFW_SPI_MAX_HOSTS    (3U)

static bool s_initialized[NFW_SPI_MAX_HOSTS];

static spi_device_handle_t s_devices[NFW_SPI_MAX_HOSTS];

/* ============================================================================
 * Internal helpers
 * ========================================================================== */

static bool nfwSpiHostIsValid(
    uint32_t host)
{
    return host < NFW_SPI_MAX_HOSTS;
}

static spi_host_device_t nfwSpiMapHost(
    uint32_t host)
{
    return (spi_host_device_t)host;
}

static NfwStatus_t nfwSpiMapError(
    esp_err_t err)
{
    if (err == ESP_OK)
    {
        return NFW_STATUS_OK;
    }

    if (err == ESP_ERR_INVALID_ARG)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (err == ESP_ERR_INVALID_STATE)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    return NFW_STATUS_ERROR;
}

/* ============================================================================
 * Public API
 * ========================================================================== */

NfwStatus_t nfwSpiInit(
    const NfwSpiConfig_t *config)
{
    spi_bus_config_t busConfig;
    spi_device_interface_config_t deviceConfig;

    esp_err_t err;

    if (config == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!nfwSpiHostIsValid(config->host))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->clockHz == 0U)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if ((config->mosiPin < 0) ||
        (config->misoPin < 0) ||
        (config->sclkPin < 0) ||
        (config->csPin < 0))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (s_initialized[config->host])
    {
        return NFW_STATUS_ALREADY_INITIALIZED;
    }

    busConfig = (spi_bus_config_t)
    {
        .mosi_io_num = config->mosiPin,
        .miso_io_num = config->misoPin,
        .sclk_io_num = config->sclkPin,

        .quadwp_io_num = -1,
        .quadhd_io_num = -1,

        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,

        .max_transfer_sz = 256
    };

    err = spi_bus_initialize(
        nfwSpiMapHost(config->host),
        &busConfig,
        SPI_DMA_CH_AUTO);

    if (err != ESP_OK)
    {
        return nfwSpiMapError(err);
    }

    /*
     * SX1262 uses SPI mode 0.
     *
     * The transport layer controls BUSY/RESET/DIO1 separately.
     */
    deviceConfig = (spi_device_interface_config_t)
    {
        .clock_speed_hz = (int)config->clockHz,
        .mode = 0,
        .spics_io_num = config->csPin,
        .queue_size = 1
    };

    err = spi_bus_add_device(
        nfwSpiMapHost(config->host),
        &deviceConfig,
        &s_devices[config->host]);

    if (err != ESP_OK)
    {
        (void)spi_bus_free(
            nfwSpiMapHost(config->host));

        s_devices[config->host] = NULL;

        return nfwSpiMapError(err);
    }

    s_initialized[config->host] = true;

    return NFW_STATUS_OK;
}

NfwStatus_t nfwSpiTransfer(
    uint32_t host,
    const uint8_t *txData,
    uint8_t *rxData,
    uint32_t length)
{
    spi_transaction_t transaction;
    esp_err_t err;

    if (!nfwSpiHostIsValid(host))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!s_initialized[host])
    {
        return NFW_STATUS_INVALID_STATE;
    }

    if (length == 0U)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if ((txData == NULL) &&
        (rxData == NULL))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    transaction = (spi_transaction_t)
    {
        .length = length * 8U,
        .rxlength = (rxData != NULL)
                  ? (length * 8U)
                  : 0U,

        .tx_buffer = txData,
        .rx_buffer = rxData
    };

    err = spi_device_transmit(
        s_devices[host],
        &transaction);

    return nfwSpiMapError(err);
}

NfwStatus_t nfwSpiDeinit(
    uint32_t host)
{
    esp_err_t err;

    if (!nfwSpiHostIsValid(host))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!s_initialized[host])
    {
        return NFW_STATUS_OK;
    }

    if (s_devices[host] != NULL)
    {
        err = spi_bus_remove_device(
            s_devices[host]);

        if (err != ESP_OK)
        {
            return nfwSpiMapError(err);
        }

        s_devices[host] = NULL;
    }

    err = spi_bus_free(
        nfwSpiMapHost(host));

    if (err != ESP_OK)
    {
        return nfwSpiMapError(err);
    }

    s_initialized[host] = false;

    return NFW_STATUS_OK;
}

bool nfwSpiIsInitialized(
    uint32_t host)
{
    if (!nfwSpiHostIsValid(host))
    {
        return false;
    }

    return s_initialized[host];
}