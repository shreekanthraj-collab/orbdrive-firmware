#ifndef NFW_FAULT_H
#define NFW_FAULT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file nfw_fault.h
 * @brief Common Orb Drive firmware fault-code contract.
 *
 * Fault codes are framework-wide identifiers and are therefore defined
 * at Core/Layer 0 rather than inside the product fault manager.
 */

typedef enum
{
    FAULT_CODE_NONE = 0,
    FAULT_CODE_LOCK,
    FAULT_CODE_VOLTAGE,
    FAULT_CODE_I2C,
    FAULT_CODE_OVERCURRENT,
    FAULT_CODE_ACTUATOR,
    FAULT_CODE_DISENGAGE
} FaultCode_t;

#ifdef __cplusplus
}
#endif

#endif /* NFW_FAULT_H */