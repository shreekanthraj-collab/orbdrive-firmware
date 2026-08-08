#include "condition_monitor.h"

#define CONDITION_WARN_V_LOW          12.0f
#define CONDITION_WARN_V_HIGH         12.4f
#define CONDITION_CUT_V               11.6f
#define CONDITION_CRITICAL_V          11.2f
#define CONDITION_RESET_V             12.0f

#define CONDITION_BYPASS_TIMEOUT_MS   120000UL

void conditionMonitorInit(ConditionMonitorContext_t *context)
{
    if (context == NULL)
    {
        return;
    }

    context->voltage_state = VOLTAGE_STATE_NORMAL;

    context->voltage_bypass_start_ms = 0U;

    context->overcurrent_retry_count = 0U;
    context->overcurrent_trip_ms = 0U;

    context->actuator_last_movement_ms = 0U;
    context->disengagement_start_ms = 0U;

    context->actuator_fault_active = false;
    context->disengagement_fault_active = false;
}
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
            if (voltage_v < CONDITION_CRITICAL_V)
            {
                context->voltage_state = VOLTAGE_STATE_LOCKED;
            }
            else if (voltage_v < CONDITION_CUT_V)
            {
                context->voltage_state = VOLTAGE_STATE_WAIT_BYPASS;
                context->voltage_bypass_start_ms = now_ms;
            }
            else if ((voltage_v < CONDITION_WARN_V_LOW) ||
                     (voltage_v > CONDITION_WARN_V_HIGH))
            {
                context->voltage_state = VOLTAGE_STATE_WARNING;
            }

            break;
        }

        case VOLTAGE_STATE_WARNING:
        {
            if (voltage_v < CONDITION_CRITICAL_V)
            {
                context->voltage_state = VOLTAGE_STATE_LOCKED;
            }
            else if (voltage_v < CONDITION_CUT_V)
            {
                context->voltage_state = VOLTAGE_STATE_WAIT_BYPASS;
                context->voltage_bypass_start_ms = now_ms;
            }
            else if ((voltage_v >= CONDITION_WARN_V_LOW) &&
                     (voltage_v <= CONDITION_WARN_V_HIGH))
            {
                context->voltage_state = VOLTAGE_STATE_NORMAL;
            }

            break;
        }

        case VOLTAGE_STATE_WAIT_BYPASS:
        {
            if (voltage_v < CONDITION_CRITICAL_V)
            {
                context->voltage_state = VOLTAGE_STATE_LOCKED;
            }
            else if (voltage_v >= CONDITION_RESET_V)
            {
                context->voltage_state = VOLTAGE_STATE_NORMAL;
                context->voltage_bypass_start_ms = 0U;
            }
            else if ((now_ms - context->voltage_bypass_start_ms) >=
                     CONDITION_BYPASS_TIMEOUT_MS)
            {
                context->voltage_state = VOLTAGE_STATE_LOCKED;
            }

            break;
        }

        case VOLTAGE_STATE_BYPASS_ACTIVE:
        {
            if (voltage_v < CONDITION_CRITICAL_V)
            {
                context->voltage_state = VOLTAGE_STATE_LOCKED;
            }
            else if (voltage_v >= CONDITION_RESET_V)
            {
                context->voltage_state = VOLTAGE_STATE_NORMAL;
                context->voltage_bypass_start_ms = 0U;
            }

            break;
        }

        case VOLTAGE_STATE_LOCKED:
        {
            if (voltage_v >= CONDITION_RESET_V)
            {
                context->voltage_state = VOLTAGE_STATE_NORMAL;
                context->voltage_bypass_start_ms = 0U;
            }

            break;
        }

        default:
        {
            context->voltage_state = VOLTAGE_STATE_LOCKED;
            break;
        }
    }

    return context->voltage_state;
}