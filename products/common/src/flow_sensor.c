/**
 * @file flow_sensor.c
 * @brief Generic dual flow-sensor implementation.
 */

#include "flow_sensor.h"

#include <stddef.h>
#include <string.h>

/* ============================================================================
 * Internal helpers
 * ========================================================================== */

static bool flowSensorChannelValid(
    FlowSensorChannel_t channel)
{
    return channel < FLOW_SENSOR_COUNT;
}

/* ============================================================================
 * Initialization
 * ========================================================================== */

void flowSensorManagerInit(
    FlowSensorManager_t *manager)
{
    if (manager == NULL)
    {
        return;
    }

    memset(
        manager,
        0,
        sizeof(*manager));

    /*
     * Sensor configuration intentionally remains disabled/TBD.
     *
     * The actual sensor make/model, Modbus slave ID, register map,
     * communication parameters and scaling will be configured later
     * from the selected sensor datasheet.
     */
}

/* ============================================================================
 * Configuration
 * ========================================================================== */

NfwStatus_t flowSensorConfigure(
    FlowSensorManager_t *manager,
    FlowSensorChannel_t channel,
    const FlowSensorConfig_t *config)
{
    if (manager == NULL ||
        config == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!flowSensorChannelValid(channel))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->enabled)
    {
        if (config->modbus.slave_id == 0U)
        {
            return NFW_STATUS_INVALID_ARGUMENT;
        }

        if (config->flow_register_count == 0U)
        {
            return NFW_STATUS_INVALID_ARGUMENT;
        }

        if (config->flow_register_count >
            MODBUS_RTU_MAX_REGISTERS)
        {
            return NFW_STATUS_INVALID_ARGUMENT;
        }
    }

    manager->sensor[channel] = *config;

    /*
     * A new configuration invalidates the previous measurement.
     */
    manager->measurement[channel].valid = false;
    manager->measurement[channel].flow_value = 0.0f;
    manager->measurement[channel].timestamp_ms = 0U;

    return NFW_STATUS_OK;
}

/* ============================================================================
 * Poll sensor
 * ========================================================================== */

NfwStatus_t flowSensorPoll(
    FlowSensorManager_t *manager,
    FlowSensorChannel_t channel,
    uint32_t now_ms)
{
    FlowSensorConfig_t *config;
    FlowSensorMeasurement_t *measurement;

    uint16_t registers[MODBUS_RTU_MAX_REGISTERS];

    NfwStatus_t status;

    if (manager == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!flowSensorChannelValid(channel))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    config =
        &manager->sensor[channel];

    measurement =
        &manager->measurement[channel];

    /*
     * Disabled/TBD sensors are not polled.
     */
    if (!config->enabled)
    {
        measurement->valid = false;

        return NFW_STATUS_NOT_SUPPORTED;
    }

    memset(
        registers,
        0,
        sizeof(registers));

    status = modbusRtuReadHoldingRegisters(
        &config->modbus,
        config->flow_register,
        config->flow_register_count,
        registers);

    if (status != NFW_STATUS_OK)
    {
        measurement->valid = false;

        return status;
    }

    /*
     * The actual sensor data representation is deliberately not assumed.
     *
     * Until the sensor datasheet is available, the generic implementation
     * uses the first returned 16-bit register as the raw flow value.
     *
     * Later this can be changed to:
     *
     * - 32-bit integer
     * - IEEE-754 float
     * - multiple registers
     * - signed value
     * - vendor-specific scaling
     *
     * without changing the RS-485 or Modbus transport layers.
     */
    measurement->flow_value =
        ((float)registers[0] *
         config->flow_scale) +
        config->flow_offset;

    measurement->timestamp_ms =
        now_ms;

    measurement->valid =
        true;

    return NFW_STATUS_OK;
}

/* ============================================================================
 * Get latest measurement
 * ========================================================================== */

NfwStatus_t flowSensorGetMeasurement(
    const FlowSensorManager_t *manager,
    FlowSensorChannel_t channel,
    FlowSensorMeasurement_t *measurement)
{
    if ((manager == NULL) ||
        (measurement == NULL))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (!flowSensorChannelValid(channel))
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    *measurement =
        manager->measurement[channel];

    if (!measurement->valid)
    {
        return NFW_STATUS_NOT_FOUND;
    }

    return NFW_STATUS_OK;
}