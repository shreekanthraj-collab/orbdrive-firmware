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

typedef enum
{
    OC_OPERATION_NONE = 0,
    OC_OPERATION_OPEN,
    OC_OPERATION_CLOSE

} OcOperation_t;

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
    /*
     * EOL/Gateway/NVS configurable voltage protection.
     */
    float voltage_warning_low_v;
    float voltage_warning_high_v;
    float voltage_cutoff_bypass_v;
    float voltage_critical_v;
    float voltage_reset_v;

    uint32_t voltage_bypass_timeout_ms;

    /*
     * Gateway/NVS configurable OC limits.
     */
    float overcurrent_base_threshold_a;
    float overcurrent_active_threshold_a;

    float overcurrent_min_safe_current_a;
    float overcurrent_max_safe_current_a;

    /*
     * Current escalation level.
     *
     * Starts at minimum safe current and increases by +1 A
     * after all three attempts at the current level fail.
     */
    float overcurrent_current_level_a;

    /*
     * Retry state.
     *
     * Three attempts are allowed at every current level.
     */
    uint8_t overcurrent_retry_count;
    uint8_t overcurrent_attempt_count;

    uint32_t overcurrent_trip_ms;

    /*
     * Requested valve operation.
     *
     * The same OC protection sequence is used for both
     * opening and closing.
     */
    OcOperation_t overcurrent_operation;

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