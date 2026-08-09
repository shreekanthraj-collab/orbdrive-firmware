/**
 * @file fault_manager.h
 * @brief Orb Drive product fault manager interface.
 */

#ifndef FAULT_MANAGER_H
#define FAULT_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "device_types.h"

typedef struct
{
    FaultCode_t active_fault;
    bool fault_latched;
} FaultManagerContext_t;

void faultManagerInit(
    FaultManagerContext_t *context
);

void faultManagerReport(
    FaultManagerContext_t *context,
    FaultCode_t fault
);

void faultManagerClear(
    FaultManagerContext_t *context,
    FaultCode_t fault
);

FaultCode_t faultManagerGetActive(
    const FaultManagerContext_t *context
);

#ifdef __cplusplus
}
#endif

#endif /* FAULT_MANAGER_H */