/**
 * @file condition_monitor.c
 * @brief Orb Drive product condition monitoring implementation.
 */

#include "condition_monitor.h"

#include <stddef.h>

#include "voltage_config.h"

/* ============================================================================
 * Fixed OC retry timing
 * ========================================================================== */

#define CONDITION_OC_ATTEMPTS_PER_LEVEL    (3U)
#define CONDITION_OC_RETRY_DELAY_MS        (30000UL)

/* ============================================================================
 * Initialization
 * ========================================================================== */

void conditionMonitorInit(
    ConditionMonitorContext_t *context)
{
    VoltageConfig_t voltageConfig;

    if (context == NULL)
    {
        return;
    }

    context->voltage_state =
        VOLTAGE_STATE_NORMAL;

    context->voltage_bypass_start_ms = 0U;

    /*
     * Start with firmware voltage defaults.
     */
    context->voltage_warning_low_v =
        VOLTAGE_CONFIG_DEFAULT_WARNING_LOW_V;

    context->voltage_warning_high_v =
        VOLTAGE_CONFIG_DEFAULT_WARNING_HIGH_V;

    context->voltage_cutoff_bypass_v =
        VOLTAGE_CONFIG_DEFAULT_CUTOFF_BYPASS_V;

    context->voltage_critical_v =
        VOLTAGE_CONFIG_DEFAULT_CRITICAL_V;

    context->voltage_reset_v =
        VOLTAGE_CONFIG_DEFAULT_RESET_V;

    context->voltage_bypass_timeout_ms =
        VOLTAGE_CONFIG_DEFAULT_BYPASS_TIMEOUT_MS;

    /*
     * If voltage configuration has already been initialized,
     * use the active EOL/NVS/Gateway configuration.
     */
    if (voltageConfigGet(&voltageConfig) == NFW_STATUS_OK)
    {
        context->voltage_warning_low_v =
            voltageConfig.warningLowV;

        context->voltage_warning_high_v =
            voltageConfig.warningHighV;

        context->voltage_cutoff_bypass_v =
            voltageConfig.cutoffBypassV;

        context->voltage_critical_v =
            voltageConfig.criticalV;

        context->voltage_reset_v =
            voltageConfig.resetV;

        context->voltage_bypass_timeout_ms =
            voltageConfig.bypassTimeoutMs;
    }

    /*
     * OC configuration starts from firmware defaults.
     *
     * The active EOL/Gateway configuration will be loaded
     * when the OC configuration integration is completed.
     */
    context->overcurrent_base_threshold_a =
        1.0f;

    context->overcurrent_active_threshold_a =
        1.0f;

    context->overcurrent_min_safe_current_a =
        1.0f;

    context->overcurrent_max_safe_current_a =
        5.0f;

    context->overcurrent_current_level_a =
        1.0f;

    context->overcurrent_retry_count = 0U;

    context->overcurrent_attempt_count = 0U;

    context->overcurrent_trip_ms = 0U;

    context->overcurrent_operation =
        OC_OPERATION_NONE;

    context->last_overcurrent = false;

    context->overcurrent_fault_active = false;

    context->force_close_pending = false;
}

/* ============================================================================
 * OC threshold API
 * ========================================================================== */

void conditionMonitorSetOvercurrentThreshold(
    ConditionMonitorContext_t *context,
    float threshold_a)
{
    if (context == NULL)
    {
        return;
    }

    if (threshold_a <= 0.0f)
    {
        return;
    }

    context->overcurrent_base_threshold_a =
        threshold_a;

    context->overcurrent_active_threshold_a =
        threshold_a;
}

/* ============================================================================
 * Voltage monitoring
 * ========================================================================== */

static VoltageConditionState_t conditionMonitorEvaluateVoltage(
    ConditionMonitorContext_t *context,
    float voltage_v,
    uint32_t now_ms)
{
    if (context == NULL)
    {
        return VOLTAGE_STATE_LOCKED;
    }

    switch (context->voltage_state)
    {
        case VOLTAGE_STATE_NORMAL:
        {
            if (voltage_v <
                context->voltage_critical_v)
            {
                context->voltage_state =
                    VOLTAGE_STATE_LOCKED;
            }
            else if (voltage_v <
                     context->voltage_cutoff_bypass_v)
            {
                context->voltage_state =
                    VOLTAGE_STATE_WAIT_BYPASS;

                context->voltage_bypass_start_ms =
                    now_ms;
            }
            else if ((voltage_v <
                      context->voltage_warning_low_v) ||
                     (voltage_v >
                      context->voltage_warning_high_v))
            {
                context->voltage_state =
                    VOLTAGE_STATE_WARNING;
            }

            break;
        }

        case VOLTAGE_STATE_WARNING:
        {
            if (voltage_v <
                context->voltage_critical_v)
            {
                context->voltage_state =
                    VOLTAGE_STATE_LOCKED;
            }
            else if (voltage_v <
                     context->voltage_cutoff_bypass_v)
            {
                context->voltage_state =
                    VOLTAGE_STATE_WAIT_BYPASS;

                context->voltage_bypass_start_ms =
                    now_ms;
            }
            else if ((voltage_v >=
                      context->voltage_warning_low_v) &&
                     (voltage_v <=
                      context->voltage_warning_high_v))
            {
                context->voltage_state =
                    VOLTAGE_STATE_NORMAL;
            }

            break;
        }

        case VOLTAGE_STATE_WAIT_BYPASS:
        {
            if (voltage_v <
                context->voltage_critical_v)
            {
                context->voltage_state =
                    VOLTAGE_STATE_LOCKED;
            }
            else if (voltage_v >=
                     context->voltage_reset_v)
            {
                context->voltage_state =
                    VOLTAGE_STATE_NORMAL;

                context->voltage_bypass_start_ms =
                    0U;
            }
            else if ((now_ms -
                      context->voltage_bypass_start_ms) >=
                     context->voltage_bypass_timeout_ms)
            {
                context->voltage_state =
                    VOLTAGE_STATE_LOCKED;
            }

            break;
        }

        case VOLTAGE_STATE_BYPASS_ACTIVE:
        {
            if (voltage_v <
                context->voltage_critical_v)
            {
                context->voltage_state =
                    VOLTAGE_STATE_LOCKED;
            }
            else if (voltage_v >=
                     context->voltage_reset_v)
            {
                context->voltage_state =
                    VOLTAGE_STATE_NORMAL;

                context->voltage_bypass_start_ms =
                    0U;
            }

            break;
        }

        case VOLTAGE_STATE_LOCKED:
        {
            if (voltage_v >=
                context->voltage_reset_v)
            {
                context->voltage_state =
                    VOLTAGE_STATE_NORMAL;

                context->voltage_bypass_start_ms =
                    0U;
            }

            break;
        }

        default:
        {
            context->voltage_state =
                VOLTAGE_STATE_LOCKED;

            break;
        }
    }

    return context->voltage_state;
}

/* ============================================================================
 * OC monitoring
 * ========================================================================== */

static void conditionMonitorEvaluateOvercurrent(
    ConditionMonitorContext_t *context,
    const ConditionMonitorInput_t *input,
    uint32_t now_ms)
{
    if ((context == NULL) ||
        (input == NULL))
    {
        return;
    }

    /*
     * A final OC fault is latched.
     */
    if (context->overcurrent_fault_active)
    {
        return;
    }

    /*
     * No active OC retry timer.
     */
    if (!context->last_overcurrent)
    {
        if (input->current_a >
            context->overcurrent_active_threshold_a)
        {
            context->last_overcurrent = true;

            context->overcurrent_trip_ms =
                now_ms;

            context->overcurrent_attempt_count++;

            return;
        }

        return;
    }

    /*
     * Current OC attempt is active.
     *
     * Keep the motor stopped until the 30-second
     * retry interval expires.
     */
    if ((now_ms -
         context->overcurrent_trip_ms) <
        CONDITION_OC_RETRY_DELAY_MS)
    {
        return;
    }

    /*
     * The current level failed after the 30-second interval.
     */
    context->last_overcurrent = false;

    /*
     * Three attempts are allowed at every current level.
     */
    if (context->overcurrent_attempt_count <
        CONDITION_OC_ATTEMPTS_PER_LEVEL)
    {
        /*
         * Start the next attempt at the same current level.
         */
        if (input->current_a >
            context->overcurrent_active_threshold_a)
        {
            context->last_overcurrent = true;

            context->overcurrent_trip_ms =
                now_ms;

            context->overcurrent_attempt_count++;

            return;
        }

        /*
         * Current is no longer above the threshold.
         *
         * Operation can continue.
         */
        context->overcurrent_attempt_count = 0U;

        return;
    }

    /*
     * Three attempts failed.
     *
     * Move to the next current level by +1 A.
     */
    if ((context->overcurrent_current_level_a + 1.0f) <=
        context->overcurrent_max_safe_current_a)
    {
        context->overcurrent_current_level_a +=
            1.0f;

        context->overcurrent_active_threshold_a =
            context->overcurrent_current_level_a;

        context->overcurrent_attempt_count = 0U;

        /*
         * The next attempt starts at the new current level.
         */
        if (input->current_a >
            context->overcurrent_active_threshold_a)
        {
            context->last_overcurrent = true;

            context->overcurrent_trip_ms =
                now_ms;

            context->overcurrent_attempt_count = 1U;
        }

        return;
    }

    /*
     * Maximum safe current was reached and all three
     * attempts failed.
     *
     * This is the final fault/lock condition.
     */
    context->overcurrent_fault_active = true;

    context->force_close_pending = true;
}

/* ============================================================================
 * Result construction
 * ========================================================================== */

static void conditionMonitorBuildVoltageResult(
    VoltageConditionState_t voltage_state,
    const ConditionMonitorContext_t *context,
    const ConditionMonitorInput_t *input,
    ConditionMonitorResult_t *result)
{
    if ((context == NULL) ||
        (input == NULL) ||
        (result == NULL))
    {
        return;
    }

    result->voltage_state = voltage_state;

    result->fault =
        FAULT_CODE_NONE;

    result->motor_stop_required =
        false;

    result->lock_required =
        false;

    result->force_close_required =
        false;

    /*
     * Final OC fault.
     */
    if (context->force_close_pending)
    {
        result->state =
            CONDITION_STATE_FAULT;

        result->fault =
            FAULT_CODE_OVERCURRENT;

        result->motor_stop_required =
            true;

        if (!input->forced_close_complete)
        {
            result->force_close_required =
                true;

            return;
        }

        result->state =
            CONDITION_STATE_LOCKED;

        result->fault =
            FAULT_CODE_OVERCURRENT;

        result->motor_stop_required =
            true;

        result->lock_required =
            true;

        return;
    }

    /*
     * Active OC retry.
     */
    if (context->last_overcurrent)
    {
        result->state =
            CONDITION_STATE_FAULT;

        result->fault =
            FAULT_CODE_OVERCURRENT;

        result->motor_stop_required =
            true;

        return;
    }

    switch (voltage_state)
    {
        case VOLTAGE_STATE_NORMAL:
            result->state =
                CONDITION_STATE_NORMAL;
            break;

        case VOLTAGE_STATE_WARNING:
            result->state =
                CONDITION_STATE_WARNING;
            break;

        case VOLTAGE_STATE_WAIT_BYPASS:
            result->state =
                CONDITION_STATE_FAULT;

            result->fault =
                FAULT_CODE_VOLTAGE;

            result->motor_stop_required =
                true;
            break;

        case VOLTAGE_STATE_BYPASS_ACTIVE:
            result->state =
                CONDITION_STATE_WARNING;

            result->fault =
                FAULT_CODE_VOLTAGE;
            break;

        case VOLTAGE_STATE_LOCKED:
            result->state =
                CONDITION_STATE_LOCKED;

            result->fault =
                FAULT_CODE_LOCK;

            result->motor_stop_required =
                true;

            result->lock_required =
                true;
            break;

        default:
            result->state =
                CONDITION_STATE_LOCKED;

            result->fault =
                FAULT_CODE_LOCK;

            result->motor_stop_required =
                true;

            result->lock_required =
                true;
            break;
    }
}

/* ============================================================================
 * Main evaluation
 * ========================================================================== */

void conditionMonitorEvaluate(
    ConditionMonitorContext_t *context,
    const ConditionMonitorInput_t *input,
    uint32_t now_ms,
    ConditionMonitorResult_t *result)
{
    VoltageConditionState_t voltage_state;

    if ((context == NULL) ||
        (input == NULL) ||
        (result == NULL))
    {
        return;
    }

    voltage_state =
        conditionMonitorEvaluateVoltage(
            context,
            input->voltage_v,
            now_ms);

    conditionMonitorEvaluateOvercurrent(
        context,
        input,
        now_ms);

    conditionMonitorBuildVoltageResult(
        voltage_state,
        context,
        input,
        result);
}