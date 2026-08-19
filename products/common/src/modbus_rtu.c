/**
 * @file modbus_rtu.c
 * @brief Generic Modbus RTU master implementation.
 */

#include "modbus_rtu.h"

#include <stddef.h>

#include "rs485.h"

/* ============================================================================
 * Modbus RTU constants
 * ========================================================================== */

#define MODBUS_FUNCTION_READ_HOLDING_REGISTERS    (0x03U)

#define MODBUS_RTU_REQUEST_LENGTH                 (8U)
#define MODBUS_RTU_RESPONSE_HEADER_LENGTH         (3U)
#define MODBUS_RTU_CRC_LENGTH                     (2U)

/* ============================================================================
 * CRC-16
 * ========================================================================== */

uint16_t modbusRtuCalculateCrc(
    const uint8_t *data,
    uint32_t length)
{
    uint16_t crc = 0xFFFFU;

    if (data == NULL)
    {
        return 0U;
    }

    for (uint32_t index = 0U; index < length; index++)
    {
        crc ^= data[index];

        for (uint8_t bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x0001U) != 0U)
            {
                crc >>= 1U;
                crc ^= 0xA001U;
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return crc;
}

/* ============================================================================
 * Read holding registers
 * ========================================================================== */

NfwStatus_t modbusRtuReadHoldingRegisters(
    const ModbusRtuDeviceConfig_t *device,
    uint16_t start_register,
    uint16_t register_count,
    uint16_t *registers)
{
    uint8_t request[MODBUS_RTU_REQUEST_LENGTH];
    uint8_t response[MODBUS_RTU_MAX_FRAME_LENGTH];

    uint32_t bytes_read = 0U;
    uint32_t expected_length;

    uint16_t request_crc;
    uint16_t response_crc;

    NfwStatus_t status;

    if ((device == NULL) ||
        (registers == NULL))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (device->slave_id == 0U)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if ((register_count == 0U) ||
        (register_count > MODBUS_RTU_MAX_REGISTERS))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    /*
     * Modbus function 03 request:
     *
     * Byte 0: Slave address
     * Byte 1: Function
     * Byte 2: Start address MSB
     * Byte 3: Start address LSB
     * Byte 4: Quantity MSB
     * Byte 5: Quantity LSB
     * Byte 6: CRC LSB
     * Byte 7: CRC MSB
     */

    request[0] = device->slave_id;

    request[1] =
        MODBUS_FUNCTION_READ_HOLDING_REGISTERS;

    request[2] =
        (uint8_t)((start_register >> 8U) & 0xFFU);

    request[3] =
        (uint8_t)(start_register & 0xFFU);

    request[4] =
        (uint8_t)((register_count >> 8U) & 0xFFU);

    request[5] =
        (uint8_t)(register_count & 0xFFU);

    request_crc =
        modbusRtuCalculateCrc(
            request,
            6U);

    /*
     * Modbus RTU transmits CRC low byte first.
     */

    request[6] =
        (uint8_t)(request_crc & 0xFFU);

    request[7] =
        (uint8_t)((request_crc >> 8U) & 0xFFU);

    status =
        rs485Transmit(
            request,
            sizeof(request),
            device->response_timeout_ms);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    /*
     * Normal response:
     *
     * Slave ID
     * Function
     * Byte count
     * Register data
     * CRC
     */

    expected_length =
        MODBUS_RTU_RESPONSE_HEADER_LENGTH +
        ((uint32_t)register_count * 2U) +
        MODBUS_RTU_CRC_LENGTH;

    if (expected_length > MODBUS_RTU_MAX_FRAME_LENGTH)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    status =
        rs485Receive(
            response,
            expected_length,
            &bytes_read);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    /*
     * A complete Modbus response is required.
     */

    if (bytes_read != expected_length)
    {
        return NFW_STATUS_TIMEOUT;
    }

    /*
     * Validate slave address.
     */

    if (response[0] != device->slave_id)
    {
        return NFW_STATUS_ERROR;
    }

    /*
     * Validate function code.
     */

    if (response[1] !=
        MODBUS_FUNCTION_READ_HOLDING_REGISTERS)
    {
        /*
         * Exception response normally has function code 0x83.
         * Generic error is returned for now.
         */

        return NFW_STATUS_ERROR;
    }

    /*
     * Validate byte count.
     */

    if (response[2] !=
        (uint8_t)(register_count * 2U))
    {
        return NFW_STATUS_ERROR;
    }

    /*
     * Validate CRC.
     */

    response_crc =
        (uint16_t)response[bytes_read - 2U] |
        ((uint16_t)response[bytes_read - 1U] << 8U);

    if (modbusRtuCalculateCrc(
            response,
            bytes_read - MODBUS_RTU_CRC_LENGTH) !=
        response_crc)
    {
        return NFW_STATUS_ERROR;
    }

    /*
     * Convert Modbus big-endian register values into uint16_t values.
     */

    for (uint16_t index = 0U;
         index < register_count;
         index++)
    {
        uint32_t offset =
            MODBUS_RTU_RESPONSE_HEADER_LENGTH +
            ((uint32_t)index * 2U);

        registers[index] =
            ((uint16_t)response[offset] << 8U) |
            (uint16_t)response[offset + 1U];
    }

    return NFW_STATUS_OK;
}