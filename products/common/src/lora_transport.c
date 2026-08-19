/**
 * @file lora_transport.c
 * @brief Orb Drive SX1262 LoRa hardware transport implementation.
 */

#include "lora_transport.h"

#include <stddef.h>
#include <string.h>

#include "nfw_delay.h"
#include "nfw_gpio.h"
#include "nfw_spi.h"
#include "orb_drive_main.h"

/* ============================================================================
 * SX1262 command opcodes
 * ========================================================================== */

#define SX1262_CMD_SET_STANDBY             (0x80U)
#define SX1262_CMD_SET_RX                  (0x82U)
#define SX1262_CMD_SET_TX                  (0x83U)

#define SX1262_CMD_SET_DIO_IRQ_PARAMS      (0x08U)
#define SX1262_CMD_GET_IRQ_STATUS          (0x12U)
#define SX1262_CMD_CLEAR_IRQ_STATUS        (0x02U)

#define SX1262_CMD_SET_DIO3_AS_TCXO_CTRL   (0x97U)

#define SX1262_CMD_SET_RF_FREQUENCY        (0x86U)
#define SX1262_CMD_SET_PACKET_TYPE         (0x8AU)
#define SX1262_CMD_SET_TX_PARAMS            (0x8EU)
#define SX1262_CMD_SET_MODULATION_PARAMS   (0x8BU)
#define SX1262_CMD_SET_PACKET_PARAMS       (0x8CU)
#define SX1262_CMD_SET_BUFFER_BASE_ADDRESS (0x8FU)

#define SX1262_CMD_WRITE_BUFFER             (0x0EU)
#define SX1262_CMD_READ_BUFFER              (0x1EU)
#define SX1262_CMD_GET_RX_BUFFER_STATUS     (0x13U)

/* ============================================================================
 * SX1262 operating values
 * ========================================================================== */

#define SX1262_PACKET_TYPE_LORA             (0x01U)
#define SX1262_STANDBY_RC                   (0x00U)

/*
 * Core1262-HF contains a TCXO controlled through DIO3.
 *
 * DIO3 voltage code:
 * 0x07 = 3.3 V
 *
 * Timeout is expressed in 15.625 us units.
 * 10 ms = 640 units.
 */
#define SX1262_TCXO_VOLTAGE_3V3             (0x07U)
#define SX1262_TCXO_TIMEOUT_10MS            (640UL)

/*
 * SPI maximum supported by the Waveshare module is 18 MHz.
 * Keep 8 MHz for a conservative first implementation.
 */
#define SX1262_SPI_CLOCK_HZ                 (8000000UL)

/*
 * Reset is specified by Waveshare as a short active-low pulse.
 * 2 ms is intentionally conservative.
 */
#define SX1262_RESET_LOW_MS                 (2U)
#define SX1262_RESET_RELEASE_MS             (10U)

#define SX1262_BUSY_POLL_MS                 (1U)
#define SX1262_COMMAND_TIMEOUT_MS           (100U)

/*
 * SX1262 FIFO access must fit inside the current SPI HAL's
 * max_transfer_sz of 256 bytes.
 *
 * WriteBuffer requires:
 *     opcode + offset + payload
 *
 * Therefore keep payload <= 254 bytes.
 */
#define LORA_MAX_PAYLOAD_LENGTH             (254U)

/*
 * LoRa packet defaults.
 */
#define SX1262_LORA_PREAMBLE_LENGTH         (8U)
#define SX1262_LORA_HEADER_EXPLICIT         (0x00U)
#define SX1262_LORA_CRC_ON                  (0x01U)
#define SX1262_LORA_IQ_STANDARD             (0x00U)

/*
 * TX power configuration.
 *
 * SX1262 high-power PA supports -9 to +22 dBm.
 */
#define SX1262_TX_POWER_MIN_DBM             (-9)
#define SX1262_TX_POWER_MAX_DBM             (22)

#define SX1262_PA_RAMP_200_US               (0x04U)

/*
 * IRQ bits.
 */
#define SX1262_IRQ_TX_DONE                  (0x0001U)
#define SX1262_IRQ_RX_DONE                  (0x0002U)
#define SX1262_IRQ_TIMEOUT                  (0x0200U)
#define SX1262_IRQ_CRC_ERROR                (0x0040U)

#define SX1262_IRQ_TX_RX_TIMEOUT_MASK       (\
    SX1262_IRQ_TX_DONE |                    \
    SX1262_IRQ_RX_DONE |                    \
    SX1262_IRQ_TIMEOUT |                    \
    SX1262_IRQ_CRC_ERROR)

/* ============================================================================
 * Internal state
 * ========================================================================== */

static bool s_initialized = false;
static uint32_t s_spi_host = 0U;

static LoraTransportConfig_t s_config;

/* ============================================================================
 * GPIO helpers
 * ========================================================================== */

static NfwStatus_t loraWritePin(
    uint32_t pin,
    bool level)
{
    return nfwGpioWrite(
        pin,
        level);
}

static NfwStatus_t loraReadPin(
    uint32_t pin,
    bool *level)
{
    return nfwGpioRead(
        pin,
        level);
}

/* ============================================================================
 * RF switch control
 *
 * Core1262-HF external RF switch:
 *
 * RECEIVE:
 *     RXEN = LOW
 *     TXEN = HIGH
 *
 * TRANSMIT:
 *     RXEN = HIGH
 *     TXEN = LOW
 *
 * Keep both disabled while changing radio state.
 * ========================================================================== */

static NfwStatus_t loraRfSwitchOff(void)
{
    NfwStatus_t status;

    status = loraWritePin(
        ORB_LORA_RXEN_GPIO,
        false);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    return loraWritePin(
        ORB_LORA_TXEN_GPIO,
        false);
}

static NfwStatus_t loraRfSwitchReceive(void)
{
    NfwStatus_t status;

    status = loraWritePin(
        ORB_LORA_TXEN_GPIO,
        false);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraWritePin(
        ORB_LORA_RXEN_GPIO,
        false);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    return loraWritePin(
        ORB_LORA_TXEN_GPIO,
        true);
}

static NfwStatus_t loraRfSwitchTransmit(void)
{
    NfwStatus_t status;

    status = loraWritePin(
        ORB_LORA_RXEN_GPIO,
        false);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraWritePin(
        ORB_LORA_TXEN_GPIO,
        false);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    return loraWritePin(
        ORB_LORA_RXEN_GPIO,
        true);
}

/* ============================================================================
 * BUSY handling
 * ========================================================================== */

NfwStatus_t loraTransportWaitReady(
    uint32_t timeoutMs)
{
    uint32_t elapsedMs = 0U;
    bool busy = false;

    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    while (elapsedMs < timeoutMs)
    {
        NfwStatus_t status;

        status = loraReadPin(
            ORB_LORA_BUSY_GPIO,
            &busy);

        if (status != NFW_STATUS_OK)
        {
            return status;
        }

        if (!busy)
        {
            return NFW_STATUS_OK;
        }

        (void)nfwDelayMs(
            SX1262_BUSY_POLL_MS);

        elapsedMs += SX1262_BUSY_POLL_MS;
    }

    return NFW_STATUS_TIMEOUT;
}

/* ============================================================================
 * Generic SX1262 command transfer
 * ========================================================================== */

static NfwStatus_t loraCommandWrite(
    const uint8_t *command,
    uint32_t length)
{
    NfwStatus_t status;

    if ((command == NULL) ||
        (length == 0U))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    status = loraTransportWaitReady(
        SX1262_COMMAND_TIMEOUT_MS);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    return nfwSpiTransfer(
        s_spi_host,
        command,
        NULL,
        length);
}

static NfwStatus_t loraCommandRead(
    const uint8_t *command,
    uint8_t *response,
    uint32_t length)
{
    NfwStatus_t status;

    if ((command == NULL) ||
        (response == NULL) ||
        (length == 0U))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    status = loraTransportWaitReady(
        SX1262_COMMAND_TIMEOUT_MS);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    return nfwSpiTransfer(
        s_spi_host,
        command,
        response,
        length);
}

/* ============================================================================
 * Frequency conversion
 * ========================================================================== */

static uint32_t loraFrequencyToPll(
    uint32_t frequencyHz)
{
    /*
     * RF frequency register:
     *
     * RFfreq = frequencyHz * 2^25 / 32 MHz
     */
    uint64_t value =
        ((uint64_t)frequencyHz * (1ULL << 25)) /
        32000000ULL;

    return (uint32_t)value;
}

/* ============================================================================
 * Bandwidth mapping
 *
 * Public transport defaults are symbolic:
 *
 * 0 -> 125 kHz
 * 1 -> 250 kHz
 * 2 -> 500 kHz
 *
 * Values 3..9 are accepted as direct SX1262 bandwidth codes.
 * ========================================================================== */

static uint8_t loraMapBandwidth(
    uint8_t bandwidth)
{
    switch (bandwidth)
    {
        case 0U:
            return 0x04U; /* 125 kHz */

        case 1U:
            return 0x05U; /* 250 kHz */

        case 2U:
            return 0x06U; /* 500 kHz */

        default:
            return bandwidth;
    }
}

/* ============================================================================
 * SX1262 commands
 * ========================================================================== */

static NfwStatus_t loraSetStandby(void)
{
    const uint8_t command[] =
    {
        SX1262_CMD_SET_STANDBY,
        SX1262_STANDBY_RC
    };

    return loraCommandWrite(
        command,
        sizeof(command));
}

static NfwStatus_t loraSetDio3Tcxo(void)
{
    const uint8_t command[] =
    {
        SX1262_CMD_SET_DIO3_AS_TCXO_CTRL,
        SX1262_TCXO_VOLTAGE_3V3,
        (uint8_t)((SX1262_TCXO_TIMEOUT_10MS >> 16) & 0xFFU),
        (uint8_t)((SX1262_TCXO_TIMEOUT_10MS >> 8) & 0xFFU),
        (uint8_t)(SX1262_TCXO_TIMEOUT_10MS & 0xFFU)
    };

    return loraCommandWrite(
        command,
        sizeof(command));
}

static NfwStatus_t loraSetPacketTypeLoRa(void)
{
    const uint8_t command[] =
    {
        SX1262_CMD_SET_PACKET_TYPE,
        SX1262_PACKET_TYPE_LORA
    };

    return loraCommandWrite(
        command,
        sizeof(command));
}

static NfwStatus_t loraSetRfFrequency(
    uint32_t frequencyHz)
{
    uint8_t command[5];
    uint32_t pll;

    pll = loraFrequencyToPll(
        frequencyHz);

    command[0] = SX1262_CMD_SET_RF_FREQUENCY;
    command[1] = (uint8_t)((pll >> 24) & 0xFFU);
    command[2] = (uint8_t)((pll >> 16) & 0xFFU);
    command[3] = (uint8_t)((pll >> 8) & 0xFFU);
    command[4] = (uint8_t)(pll & 0xFFU);

    return loraCommandWrite(
        command,
        sizeof(command));
}

static NfwStatus_t loraSetModulationParams(void)
{
    uint8_t command[5];
    uint8_t bandwidth;

    bandwidth = loraMapBandwidth(
        s_config.bandwidth);

    if ((s_config.spreadingFactor < 5U) ||
        (s_config.spreadingFactor > 12U))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if ((s_config.codingRate < 1U) ||
        (s_config.codingRate > 4U))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    /*
     * LDRO:
     *
     * 0 = disabled
     *
     * For the initial Orb Drive transport configuration,
     * keep LDRO disabled. It can be made configurable later
     * once the link profile is finalized.
     */
    command[0] = SX1262_CMD_SET_MODULATION_PARAMS;
    command[1] = s_config.spreadingFactor;
    command[2] = bandwidth;
    command[3] = s_config.codingRate;
    command[4] = 0x00U;

    return loraCommandWrite(
        command,
        sizeof(command));
}

static NfwStatus_t loraSetPacketParams(
    uint8_t payloadLength)
{
    uint8_t command[7];

    command[0] = SX1262_CMD_SET_PACKET_PARAMS;

    command[1] =
        (uint8_t)((SX1262_LORA_PREAMBLE_LENGTH >> 8) & 0xFFU);

    command[2] =
        (uint8_t)(SX1262_LORA_PREAMBLE_LENGTH & 0xFFU);

    command[3] = SX1262_LORA_HEADER_EXPLICIT;
    command[4] = payloadLength;
    command[5] = SX1262_LORA_CRC_ON;
    command[6] = SX1262_LORA_IQ_STANDARD;

    return loraCommandWrite(
        command,
        sizeof(command));
}

static NfwStatus_t loraSetBufferBaseAddress(void)
{
    const uint8_t command[] =
    {
        SX1262_CMD_SET_BUFFER_BASE_ADDRESS,
        0x00U, /* TX base */
        0x00U  /* RX base */
    };

    return loraCommandWrite(
        command,
        sizeof(command));
}

static NfwStatus_t loraSetTxParams(void)
{
    int8_t power;
    uint8_t command[3];

    power = s_config.txPowerDbm;

    if (power < SX1262_TX_POWER_MIN_DBM)
    {
        power = SX1262_TX_POWER_MIN_DBM;
    }

    if (power > SX1262_TX_POWER_MAX_DBM)
    {
        power = SX1262_TX_POWER_MAX_DBM;
    }

    command[0] = SX1262_CMD_SET_TX_PARAMS;
    command[1] = (uint8_t)power;
    command[2] = SX1262_PA_RAMP_200_US;

    return loraCommandWrite(
        command,
        sizeof(command));
}

static NfwStatus_t loraSetIrqParams(void)
{
    uint8_t command[9];
    uint16_t mask = SX1262_IRQ_TX_RX_TIMEOUT_MASK;

    command[0] = SX1262_CMD_SET_DIO_IRQ_PARAMS;

    command[1] = (uint8_t)((mask >> 8) & 0xFFU);
    command[2] = (uint8_t)(mask & 0xFFU);

    /*
     * DIO1 receives the same interrupt mask.
     */
    command[3] = (uint8_t)((mask >> 8) & 0xFFU);
    command[4] = (uint8_t)(mask & 0xFFU);

    /*
     * DIO2/DIO3 IRQ mapping not used by the transport.
     */
    command[5] = 0x00U;
    command[6] = 0x00U;
    command[7] = 0x00U;
    command[8] = 0x00U;

    return loraCommandWrite(
        command,
        sizeof(command));
}

static NfwStatus_t loraGetIrqStatus(
    uint16_t *irqStatus)
{
    uint8_t command[3] =
    {
        SX1262_CMD_GET_IRQ_STATUS,
        0x00U,
        0x00U
    };

    uint8_t response[3];

    NfwStatus_t status;

    if (irqStatus == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    memset(
        response,
        0,
        sizeof(response));

    status = loraCommandRead(
        command,
        response,
        sizeof(response));

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    /*
     * Response:
     *
     * byte 0 = status
     * byte 1 = IRQ MSB
     * byte 2 = IRQ LSB
     */
    *irqStatus =
        ((uint16_t)response[1] << 8) |
        (uint16_t)response[2];

    return NFW_STATUS_OK;
}

static NfwStatus_t loraClearIrqStatus(
    uint16_t irqStatus)
{
    uint8_t command[3];

    command[0] = SX1262_CMD_CLEAR_IRQ_STATUS;
    command[1] = (uint8_t)((irqStatus >> 8) & 0xFFU);
    command[2] = (uint8_t)(irqStatus & 0xFFU);

    return loraCommandWrite(
        command,
        sizeof(command));
}

static NfwStatus_t loraWriteBuffer(
    const uint8_t *data,
    uint32_t length)
{
    uint8_t buffer[LORA_MAX_PAYLOAD_LENGTH + 2U];

    if ((data == NULL) ||
        (length == 0U) ||
        (length > LORA_MAX_PAYLOAD_LENGTH))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    buffer[0] = SX1262_CMD_WRITE_BUFFER;
    buffer[1] = 0x00U;

    memcpy(
        &buffer[2],
        data,
        length);

    return loraCommandWrite(
        buffer,
        length + 2U);
}

static NfwStatus_t loraGetRxBufferStatus(
    uint8_t *payloadLength,
    uint8_t *startPointer)
{
    uint8_t command[3] =
    {
        SX1262_CMD_GET_RX_BUFFER_STATUS,
        0x00U,
        0x00U
    };

    uint8_t response[3];

    NfwStatus_t status;

    if ((payloadLength == NULL) ||
        (startPointer == NULL))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    memset(
        response,
        0,
        sizeof(response));

    status = loraCommandRead(
        command,
        response,
        sizeof(response));

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    /*
     * Response:
     *
     * byte 0 = status
     * byte 1 = payload length
     * byte 2 = RX start pointer
     */
    *payloadLength = response[1];
    *startPointer = response[2];

    return NFW_STATUS_OK;
}

static NfwStatus_t loraReadBuffer(
    uint8_t offset,
    uint8_t *data,
    uint32_t length)
{
    uint8_t command[
        LORA_MAX_PAYLOAD_LENGTH + 3U];

    uint8_t response[
        LORA_MAX_PAYLOAD_LENGTH + 3U];

    NfwStatus_t status;

    if ((data == NULL) ||
        (length == 0U) ||
        (length > LORA_MAX_PAYLOAD_LENGTH))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    command[0] = SX1262_CMD_READ_BUFFER;
    command[1] = offset;
    command[2] = 0x00U;

    memset(
        &command[3],
        0,
        length);

    memset(
        response,
        0,
        length + 3U);

    status = loraCommandRead(
        command,
        response,
        length + 3U);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    /*
     * Response:
     *
     * byte 0 = status
     * byte 1 = dummy
     * byte 2 = dummy
     * byte 3.. = FIFO data
     */
    memcpy(
        data,
        &response[3],
        length);

    return NFW_STATUS_OK;
}

/* ============================================================================
 * Hardware reset
 * ========================================================================== */

NfwStatus_t loraTransportReset(void)
{
    NfwStatus_t status;

    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    status = loraRfSwitchOff();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraWritePin(
        ORB_LORA_RESET_GPIO,
        false);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    (void)nfwDelayMs(
        SX1262_RESET_LOW_MS);

    status = loraWritePin(
        ORB_LORA_RESET_GPIO,
        true);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    (void)nfwDelayMs(
        SX1262_RESET_RELEASE_MS);

    return loraTransportWaitReady(
        SX1262_COMMAND_TIMEOUT_MS);
}

/* ============================================================================
 * SX1262 radio configuration
 * ========================================================================== */

static NfwStatus_t loraConfigureRadio(void)
{
    NfwStatus_t status;

    status = loraSetStandby();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraSetDio3Tcxo();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraSetPacketTypeLoRa();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraSetRfFrequency(
        s_config.frequencyHz);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraSetModulationParams();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    /*
     * Payload length is updated immediately before TX/RX because
     * explicit-header mode is being used.
     */
    status = loraSetPacketParams(
        0U);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraSetBufferBaseAddress();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraSetTxParams();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraSetIrqParams();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    return loraClearIrqStatus(
        0xFFFFU);
}

/* ============================================================================
 * Initialization
 * ========================================================================== */

NfwStatus_t loraTransportInit(
    const LoraTransportConfig_t *config)
{
    NfwSpiConfig_t spiConfig;
    NfwGpioConfig_t gpioConfig;
    NfwStatus_t status;

    if (config == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (s_initialized)
    {
        return NFW_STATUS_ALREADY_INITIALIZED;
    }

    if (config->spiHost >= 3U)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if ((config->frequencyHz < 850000000UL) ||
        (config->frequencyHz > 930000000UL))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if ((config->spreadingFactor < 5U) ||
        (config->spreadingFactor > 12U))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if ((config->codingRate < 1U) ||
        (config->codingRate > 4U))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->txPowerDbm <
            SX1262_TX_POWER_MIN_DBM ||
        config->txPowerDbm >
            SX1262_TX_POWER_MAX_DBM)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    s_config = *config;
    s_spi_host = config->spiHost;

    spiConfig = (NfwSpiConfig_t)
    {
        .host = s_spi_host,
        .clockHz = SX1262_SPI_CLOCK_HZ,
        .mosiPin = ORB_LORA_SPI_MOSI_GPIO,
        .misoPin = ORB_LORA_SPI_MISO_GPIO,
        .sclkPin = ORB_LORA_SPI_SCK_GPIO,
        .csPin = ORB_LORA_SPI_CS_GPIO
    };

    status = nfwSpiInit(
        &spiConfig);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    /*
     * RESET.
     */
    gpioConfig = (NfwGpioConfig_t)
    {
        .pin = ORB_LORA_RESET_GPIO,
        .direction = NFW_GPIO_DIRECTION_OUTPUT,
        .pull = NFW_GPIO_PULL_NONE,
        .initialLevel = true
    };

    status = nfwGpioInit(
        &gpioConfig);

    if (status != NFW_STATUS_OK)
    {
        (void)nfwSpiDeinit(
            s_spi_host);

        return status;
    }

    /*
     * RXEN.
     */
    gpioConfig.pin = ORB_LORA_RXEN_GPIO;
    gpioConfig.initialLevel = false;

    status = nfwGpioInit(
        &gpioConfig);

    if (status != NFW_STATUS_OK)
    {
        (void)nfwSpiDeinit(
            s_spi_host);

        return status;
    }

    /*
     * TXEN.
     */
    gpioConfig.pin = ORB_LORA_TXEN_GPIO;
    gpioConfig.initialLevel = false;

    status = nfwGpioInit(
        &gpioConfig);

    if (status != NFW_STATUS_OK)
    {
        (void)nfwSpiDeinit(
            s_spi_host);

        return status;
    }

    /*
     * BUSY.
     */
    gpioConfig = (NfwGpioConfig_t)
    {
        .pin = ORB_LORA_BUSY_GPIO,
        .direction = NFW_GPIO_DIRECTION_INPUT,
        .pull = NFW_GPIO_PULL_NONE,
        .initialLevel = false
    };

    status = nfwGpioInit(
        &gpioConfig);

    if (status != NFW_STATUS_OK)
    {
        (void)nfwSpiDeinit(
            s_spi_host);

        return status;
    }

    /*
     * DIO1.
     */
    gpioConfig.pin = ORB_LORA_DIO1_GPIO;

    status = nfwGpioInit(
        &gpioConfig);

    if (status != NFW_STATUS_OK)
    {
        (void)nfwSpiDeinit(
            s_spi_host);

        return status;
    }

    s_initialized = true;

    /*
     * Put RF switch in inactive state before radio reset.
     */
    status = loraRfSwitchOff();

    if (status != NFW_STATUS_OK)
    {
        (void)loraTransportDeinit();
        return status;
    }

    status = loraTransportReset();

    if (status != NFW_STATUS_OK)
    {
        (void)loraTransportDeinit();
        return status;
    }

    status = loraConfigureRadio();

    if (status != NFW_STATUS_OK)
    {
        (void)loraTransportDeinit();
        return status;
    }

    return NFW_STATUS_OK;
}

/* ============================================================================
 * Initialization status
 * ========================================================================== */

bool loraTransportIsInitialized(void)
{
    return s_initialized;
}

/* ============================================================================
 * Transmit
 * ========================================================================== */

NfwStatus_t loraTransportTransmit(
    const uint8_t *data,
    uint32_t length,
    uint32_t timeoutMs)
{
    uint16_t irqStatus;
    uint32_t elapsedMs = 0U;
    NfwStatus_t status;

    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    if ((data == NULL) ||
        (length == 0U) ||
        (length > LORA_MAX_PAYLOAD_LENGTH))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    status = loraTransportStandby();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraRfSwitchOff();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraClearIrqStatus(
        0xFFFFU);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraSetPacketParams(
        (uint8_t)length);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraWriteBuffer(
        data,
        length);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraRfSwitchTransmit();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    /*
     * SetTx timeout is in 15.625 us units.
     */
    {
        uint32_t txTimeout;

        if (timeoutMs == 0U)
        {
            txTimeout = 0xFFFFFFUL;
        }
        else
        {
            uint64_t calculated =
                (uint64_t)timeoutMs * 64ULL;

            if (calculated > 0xFFFFFFUL)
            {
                calculated = 0xFFFFFFUL;
            }

            txTimeout = (uint32_t)calculated;
        }

        uint8_t command[4];

        command[0] = SX1262_CMD_SET_TX;
        command[1] =
            (uint8_t)((txTimeout >> 16) & 0xFFU);
        command[2] =
            (uint8_t)((txTimeout >> 8) & 0xFFU);
        command[3] =
            (uint8_t)(txTimeout & 0xFFU);

        status = loraCommandWrite(
            command,
            sizeof(command));
    }

    if (status != NFW_STATUS_OK)
    {
        (void)loraRfSwitchOff();
        return status;
    }

    while (elapsedMs < timeoutMs)
    {
        bool dio1 = false;

        status = loraReadPin(
            ORB_LORA_DIO1_GPIO,
            &dio1);

        if (status != NFW_STATUS_OK)
        {
            (void)loraRfSwitchOff();
            return status;
        }

        if (dio1)
        {
            status = loraGetIrqStatus(
                &irqStatus);

            if (status != NFW_STATUS_OK)
            {
                (void)loraRfSwitchOff();
                return status;
            }

            (void)loraClearIrqStatus(
                irqStatus);

            (void)loraRfSwitchOff();

            if ((irqStatus & SX1262_IRQ_TX_DONE) != 0U)
            {
                return NFW_STATUS_OK;
            }

            if ((irqStatus & SX1262_IRQ_TIMEOUT) != 0U)
            {
                return NFW_STATUS_TIMEOUT;
            }

            return NFW_STATUS_ERROR;
        }

        (void)nfwDelayMs(
            SX1262_BUSY_POLL_MS);

        elapsedMs += SX1262_BUSY_POLL_MS;
    }

    (void)loraTransportStandby();
    (void)loraRfSwitchOff();

    return NFW_STATUS_TIMEOUT;
}

/* ============================================================================
 * Start receive
 * ========================================================================== */

NfwStatus_t loraTransportStartReceive(void)
{
    const uint8_t command[] =
    {
        SX1262_CMD_SET_RX,
        0xFFU,
        0xFFU,
        0xFFU
    };

    NfwStatus_t status;

    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    status = loraTransportStandby();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraRfSwitchOff();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraClearIrqStatus(
        0xFFFFU);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    /*
     * Payload length is ignored by explicit-header mode on RX.
     */
    status = loraSetPacketParams(
        0U);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = loraRfSwitchReceive();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    return loraCommandWrite(
        command,
        sizeof(command));
}

/* ============================================================================
 * Receive
 * ========================================================================== */

NfwStatus_t loraTransportReceive(
    uint8_t *data,
    uint32_t maxLength,
    uint32_t *receivedLength,
    uint32_t timeoutMs)
{
    uint32_t elapsedMs = 0U;
    uint16_t irqStatus;
    uint8_t payloadLength;
    uint8_t startPointer;
    NfwStatus_t status;

    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    if ((data == NULL) ||
        (receivedLength == NULL) ||
        (maxLength == 0U) ||
        (maxLength > LORA_MAX_PAYLOAD_LENGTH))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    *receivedLength = 0U;

    status = loraTransportStartReceive();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    while (elapsedMs < timeoutMs)
    {
        bool dio1 = false;

        status = loraReadPin(
            ORB_LORA_DIO1_GPIO,
            &dio1);

        if (status != NFW_STATUS_OK)
        {
            (void)loraTransportStandby();
            (void)loraRfSwitchOff();
            return status;
        }

        if (dio1)
        {
            status = loraGetIrqStatus(
                &irqStatus);

            if (status != NFW_STATUS_OK)
            {
                (void)loraTransportStandby();
                (void)loraRfSwitchOff();
                return status;
            }

            (void)loraClearIrqStatus(
                irqStatus);

            if ((irqStatus & SX1262_IRQ_CRC_ERROR) != 0U)
            {
                (void)loraTransportStandby();
                (void)loraRfSwitchOff();

                return NFW_STATUS_ERROR;
            }

            if ((irqStatus & SX1262_IRQ_RX_DONE) != 0U)
            {
                status = loraGetRxBufferStatus(
                    &payloadLength,
                    &startPointer);

                if (status != NFW_STATUS_OK)
                {
                    (void)loraTransportStandby();
                    (void)loraRfSwitchOff();
                    return status;
                }

                if (payloadLength > maxLength)
                {
                    (void)loraTransportStandby();
                    (void)loraRfSwitchOff();

                    return NFW_STATUS_INVALID_ARGUMENT;
                }

                status = loraReadBuffer(
                    startPointer,
                    data,
                    payloadLength);

                (void)loraTransportStandby();
                (void)loraRfSwitchOff();

                if (status != NFW_STATUS_OK)
                {
                    return status;
                }

                *receivedLength =
                    payloadLength;

                return NFW_STATUS_OK;
            }

            if ((irqStatus & SX1262_IRQ_TIMEOUT) != 0U)
            {
                (void)loraTransportStandby();
                (void)loraRfSwitchOff();

                return NFW_STATUS_TIMEOUT;
            }

            (void)loraTransportStandby();
            (void)loraRfSwitchOff();

            return NFW_STATUS_ERROR;
        }

        (void)nfwDelayMs(
            SX1262_BUSY_POLL_MS);

        elapsedMs += SX1262_BUSY_POLL_MS;
    }

    (void)loraTransportStandby();
    (void)loraRfSwitchOff();

    return NFW_STATUS_TIMEOUT;
}

/* ============================================================================
 * Standby
 * ========================================================================== */

NfwStatus_t loraTransportStandby(void)
{
    NfwStatus_t status;

    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    status = loraRfSwitchOff();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    return loraSetStandby();
}

/* ============================================================================
 * Deinitialize
 * ========================================================================== */

NfwStatus_t loraTransportDeinit(void)
{
    NfwStatus_t status;

    if (!s_initialized)
    {
        return NFW_STATUS_OK;
    }

    (void)loraRfSwitchOff();

    status = nfwSpiDeinit(
        s_spi_host);

    s_initialized = false;
    s_spi_host = 0U;

    memset(
        &s_config,
        0,
        sizeof(s_config));

    return status;
}