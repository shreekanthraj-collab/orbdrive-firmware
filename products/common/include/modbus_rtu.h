/**
 * @file modbus_rtu.h
 * @brief Generic Modbus RTU master interface for Orb Drive flow sensors.
 */

#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "nfw_status.h"

/* ============================================================================
 * Modbus RTU configuration
 * ========================================================================== */

typedef struct
{
    uint8_t slave_id;
    uint32_t response_timeout_ms;

} ModbusRtuDeviceConfig_t;

/* ============================================================================
 * Limits
 * ========================================================================== */

#define MODBUS_RTU_MAX_REGISTERS    (32U)
#define MODBUS_RTU_MAX_FRAME_LENGTH (256U)

/* ============================================================================
 * Public API
 * ========================================================================== */

NfwStatus_t modbusRtuReadHoldingRegisters(
    const ModbusRtuDeviceConfig_t *device,
    uint16_t start_register,
    uint16_t register_count,
    uint16_t *registers);

uint16_t modbusRtuCalculateCrc(
    const uint8_t *data,
    uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_RTU_H */