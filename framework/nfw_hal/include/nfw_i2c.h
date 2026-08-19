/**
 * @file nfw_i2c.h
 * @brief Orb Drive I2C Hardware Abstraction Layer.
 */

#ifndef NFW_I2C_H
#define NFW_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "nfw_status.h"

/* ============================================================================
 * I2C configuration
 * ========================================================================== */

typedef struct
{
    uint32_t port;
    uint32_t sdaPin;
    uint32_t sclPin;
    uint32_t frequencyHz;
} NfwI2cConfig_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

/**
 * @brief Initialize the I2C peripheral.
 *
 * @param config I2C configuration.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwI2cInit(const NfwI2cConfig_t *config);

/**
 * @brief Write bytes to an I2C device.
 *
 * @param address 7-bit I2C device address.
 * @param data Data buffer.
 * @param length Number of bytes.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwI2cWrite(
    uint8_t address,
    const uint8_t *data,
    uint32_t length);

/**
 * @brief Read bytes from an I2C device.
 *
 * @param address 7-bit I2C device address.
 * @param data Destination buffer.
 * @param length Number of bytes.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwI2cRead(
    uint8_t address,
    uint8_t *data,
    uint32_t length);

/**
 * @brief Write bytes followed by a read operation.
 *
 * Useful for register-based I2C devices such as INA226.
 *
 * @param address 7-bit I2C device address.
 * @param writeData Data to write.
 * @param writeLength Number of bytes to write.
 * @param readData Destination buffer.
 * @param readLength Number of bytes to read.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwI2cWriteRead(
    uint8_t address,
    const uint8_t *writeData,
    uint32_t writeLength,
    uint8_t *readData,
    uint32_t readLength);

/**
 * @brief Check whether an I2C device acknowledges its address.
 *
 * @param address 7-bit I2C device address.
 *
 * @return true when the device acknowledges.
 */
bool nfwI2cDevicePresent(uint8_t address);

#ifdef __cplusplus
}
#endif

#endif /* NFW_I2C_H */
