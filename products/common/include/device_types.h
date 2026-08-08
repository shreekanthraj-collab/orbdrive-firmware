
/**
 * @file device_types.h
 * @brief Common Orb Drive product-level device types.
 */

#ifndef DEVICE_TYPES_H
#define DEVICE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

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

#endif /* DEVICE_TYPES_H */
