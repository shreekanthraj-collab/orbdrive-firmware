/**
 * @file flow_sensor.h
 * @brief Generic dual flow-sensor interface for Orb Drive Node.
 *
 * Two flow sensors are connected to the Node through RS-485 / Modbus RTU.
 * Sensor-specific register maps and communication parameters remain
 * configurable/TBD until the actual sensor is selected.
 */

#ifndef FLOW_SENSOR_H
#define FLOW_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "modbus_rtu.h"
#include "nfw_status.h"

/* ============================================================================
 * Flow sensor channels
 * ========================================================================== */

typedef enum
{
    FLOW_SENSOR_1 = 0,
    FLOW_SENSOR_2,

    FLOW_SENSOR_COUNT

} FlowSensorChannel_t;

/* ============================================================================
 * Flow sensor configuration
 * ========================================================================== */

typedef struct
{
    bool enabled;

    ModbusRtuDeviceConfig_t modbus;

    /*
     * Sensor-specific register information.
     *
     * These remain TBD until the actual flow-sensor datasheet is available.
     */
    uint16_t flow_register;
    uint16_t flow_register_count;

    /*
     * Conversion from raw Modbus register value to engineering units.
     *
     * Example later:
     *
     * flow = raw_value * flow_scale + flow_offset
     */
    float flow_scale;
    float flow_offset;

} FlowSensorConfig_t;

/* ============================================================================
 * Flow sensor measurement
 * ========================================================================== */

typedef struct
{
    bool valid;

    float flow_value;

    uint32_t timestamp_ms;

} FlowSensorMeasurement_t;

/* ============================================================================
 * Flow sensor manager
 * ========================================================================== */

typedef struct
{
    FlowSensorConfig_t
        sensor[FLOW_SENSOR_COUNT];

    FlowSensorMeasurement_t
        measurement[FLOW_SENSOR_COUNT];

} FlowSensorManager_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

/**
 * @brief Initialize the flow-sensor manager.
 *
 * @param manager Flow-sensor manager context.
 */
void flowSensorManagerInit(
    FlowSensorManager_t *manager);

/**
 * @brief Configure one flow sensor.
 *
 * @param manager Flow-sensor manager context.
 * @param channel Sensor channel.
 * @param config Sensor configuration.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t flowSensorConfigure(
    FlowSensorManager_t *manager,
    FlowSensorChannel_t channel,
    const FlowSensorConfig_t *config);

/**
 * @brief Poll one configured flow sensor.
 *
 * @param manager Flow-sensor manager context.
 * @param channel Sensor channel.
 * @param now_ms Current monotonic system time.
 *
 * @return NFW_STATUS_OK on successful reading.
 */
NfwStatus_t flowSensorPoll(
    FlowSensorManager_t *manager,
    FlowSensorChannel_t channel,
    uint32_t now_ms);

/**
 * @brief Get the latest measurement from one flow sensor.
 *
 * @param manager Flow-sensor manager context.
 * @param channel Sensor channel.
 * @param measurement Destination measurement.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t flowSensorGetMeasurement(
    const FlowSensorManager_t *manager,
    FlowSensorChannel_t channel,
    FlowSensorMeasurement_t *measurement);

#ifdef __cplusplus
}
#endif

#endif /* FLOW_SENSOR_H */