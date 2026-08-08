/**
 * @file condition_monitor.h
 * @brief Orb Drive product condition monitoring interface.
 */

#ifndef CONDITION_MONITOR_H
#define CONDITION_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "device_types.h"

typedef enum
{
    CONDITION_STATE_NORMAL = 0,
    CONDITION_STATE_WARNING,
    CONDITION_STATE_FAULT,
    CONDITION_STATE_LOCKED

} ConditionState_t;

typedef enum
{
    VOLTAGE_STATE_NORMAL = 0,
    VOLTAGE_STATE_WARNING,
    VOLTAGE_STATE_WAIT_BYPASS,
    VOLTAGE_STATE_BYPASS_ACTIVE,
    VOLTAGE_STATE_LOCKED

} VoltageConditionState_t;

typedef struct
{
    float voltage_v;
    float current_a;

    float encoder_position_turns;
    float encoder_movement_turns;

    bool motor_active;
    bool current_alert;
    bool forced_close_complete;

    bool ina226_healthy;
    bool as5600_healthy;
    bool i2c_healthy;

} ConditionMonitorInput_t;

typedef struct
{
    ConditionState_t state;
    VoltageConditionState_t voltage_state;

    FaultCode_t fault;

    bool motor_stop_required;
    bool lock_required;
    bool force_close_required;

} ConditionMonitorResult_t;

typedef struct
{
    VoltageConditionState_t voltage_state;

    uint32_t voltage_bypass_start_ms;

    float overcurrent_base_threshold_a;
    float overcurrent_active_threshold_a;

    uint8_t overcurrent_retry_count;
    uint32_t overcurrent_trip_ms;

    bool last_overcurrent;
    bool overcurrent_fault_active;
    bool force_close_pending;

   } ConditionMonitorContext_t;

 /**
 * @brief Initialize the condition monitor context.
 *
 * @param context Condition monitor context.
 */
void conditionMonitorInit(
    ConditionMonitorContext_t *context
);

/**
 * @brief Set the configured overcurrent base threshold.
 *
 * @param context Condition monitor context.
 * @param threshold_a Base overcurrent threshold in amperes.
 */
void conditionMonitorSetOvercurrentThreshold(
    ConditionMonitorContext_t *context,
    float threshold_a
);

/**
 * @brief Evaluate current device conditions.
 *
 * @param context Condition monitor context.
 * @param input Current measurements and health information.
 * @param now_ms Current monotonic system time in milliseconds.
 * @param result Evaluation result.
 */
void conditionMonitorEvaluate(
    ConditionMonitorContext_t *context,
    const ConditionMonitorInput_t *input,
    uint32_t now_ms,
    ConditionMonitorResult_t *result
);


#ifdef __cplusplus
}
#endif

#endif /* CONDITION_MONITOR_H */