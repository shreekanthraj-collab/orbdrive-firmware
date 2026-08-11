#ifndef RS485_H
#define RS485_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "nfw_status.h"

/* ============================================================================
 * RS-485 Configuration
 * ========================================================================== */

typedef struct
{
    uint32_t uart_port;
    uint32_t baud_rate;
    uint8_t data_bits;
    uint8_t stop_bits;
    bool parity_enabled;

    int32_t tx_pin;
    int32_t rx_pin;
    int32_t direction_pin;
} Rs485Config_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

/**
 * @brief Initialize the RS-485 interface.
 *
 * Configures the UART and the RS-485 transmit-enable direction GPIO.
 *
 * @param config RS-485 configuration.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t rs485Init(const Rs485Config_t *config);

/**
 * @brief Transmit an RS-485 frame.
 *
 * The direction pin is asserted before transmission and released only after
 * UART transmission has completely finished.
 *
 * @param data Frame buffer.
 * @param length Frame length in bytes.
 * @param timeout_ms Maximum TX completion wait time.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t rs485Transmit(
    const uint8_t *data,
    uint32_t length,
    uint32_t timeout_ms);

/**
 * @brief Receive bytes from the RS-485 UART.
 *
 * @param data Destination buffer.
 * @param length Maximum number of bytes to receive.
 * @param bytes_read Number of bytes actually received.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t rs485Receive(
    uint8_t *data,
    uint32_t length,
    uint32_t *bytes_read);

/**
 * @brief Deinitialize the RS-485 interface.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t rs485Deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* RS485_H */