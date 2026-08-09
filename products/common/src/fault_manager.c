#include "fault_manager.h"
#include <stdint.h>
#include <stddef.h>

static bool faultManagerIsLatched(FaultCode_t fault)
{
    switch (fault)
    {
        case FAULT_CODE_LOCK:
        case FAULT_CODE_I2C:
        case FAULT_CODE_OVERCURRENT:
        case FAULT_CODE_ACTUATOR:
        case FAULT_CODE_DISENGAGE:
            return true;

        case FAULT_CODE_VOLTAGE:
        case FAULT_CODE_NONE:
        default:
            return false;
    }
}

static uint8_t faultManagerPriority(FaultCode_t fault)
{
    switch (fault)
    {
        case FAULT_CODE_LOCK:
            return 7U;

        case FAULT_CODE_OVERCURRENT:
            return 6U;

        case FAULT_CODE_ACTUATOR:
            return 5U;

        case FAULT_CODE_DISENGAGE:
            return 4U;

        case FAULT_CODE_I2C:
            return 3U;

        case FAULT_CODE_VOLTAGE:
            return 2U;

        case FAULT_CODE_NONE:
        default:
            return 0U;
    }
}

void faultManagerInit(FaultManagerContext_t *context)
{
    if (context == NULL)
    {
        return;
    }

    context->active_fault = FAULT_CODE_NONE;
    context->fault_latched = false;
}

void faultManagerReport(
    FaultManagerContext_t *context,
    FaultCode_t fault)
{
    if (context == NULL)
    {
        return;
    }

    if (fault == FAULT_CODE_NONE)
    {
        return;
    }

    if (context->active_fault == FAULT_CODE_NONE)
    {
        context->active_fault = fault;
        context->fault_latched = faultManagerIsLatched(fault);
        return;
    }

    if (faultManagerPriority(fault) >
        faultManagerPriority(context->active_fault))
    {
        context->active_fault = fault;
        context->fault_latched = faultManagerIsLatched(fault);
    }
}

void faultManagerClear(
    FaultManagerContext_t *context,
    FaultCode_t fault)
{
    if (context == NULL)
    {
        return;
    }

    if (fault == FAULT_CODE_NONE)
    {
        return;
    }

    if (context->active_fault != fault)
    {
        return;
    }

    context->active_fault = FAULT_CODE_NONE;
    context->fault_latched = false;
}

FaultCode_t faultManagerGetActive(
    const FaultManagerContext_t *context)
{
    if (context == NULL)
    {
        return FAULT_CODE_NONE;
    }

    return context->active_fault;
}
