/**
 * @file motor_controller.h
 * @brief Orb Drive motor/actuator control state machine.
 */

#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "nfw_status.h"

/* ============================================================================
 * Motor direction
 * ========================================================================== */

typedef enum
{
    MOTOR_DIRECTION_NONE = 0,
    MOTOR_DIRECTION_FORWARD,
    MOTOR_DIRECTION_REVERSE
} MotorDirection_t;

/* ============================================================================
 * Motor state
 * ========================================================================== */

typedef enum
{
    MOTOR_STATE_STOPPED = 0,
    MOTOR_STATE_RAMPING_UP,
    MOTOR_STATE_RUNNING,
    MOTOR_STATE_RAMPING_DOWN
} MotorState_t;

/* ============================================================================
 * Public API
 * ========================================================================== */

/**
 * @brief Initialize motor control hardware.
 *
 * Safe initial state:
 *   Power relay    OFF
 *   Direction      OFF
 *   PWM            0%
 *   Emergency stop NOT active
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t motorControllerInit(void);

/**
 * @brief Request motor start.
 *
 * Sequence:
 *   Direction ON
 *   PWM ramp 0 -> 100%
 *   Power relay ON
 *   RUNNING
 *
 * Motor start is rejected while the software emergency-stop
 * latch is active.
 *
 * @param direction Requested motor direction.
 *
 * @return NFW_STATUS_OK on success.
 * @return NFW_STATUS_INVALID_ARGUMENT for invalid direction.
 * @return NFW_STATUS_INVALID_STATE when emergency stop is active
 *         or motor cannot be started.
 */
NfwStatus_t motorControllerStart(MotorDirection_t direction);

/**
 * @brief Request motor stop.
 *
 * Normal controlled stop sequence:
 *   Power relay OFF
 *   PWM ramp 100 -> 0%
 *   PWM OFF
 *   Direction OFF
 *
 * This is NOT an emergency stop.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t motorControllerStop(void);

/**
 * @brief Execute an immediate software emergency stop.
 *
 * Emergency stop has priority over normal motor operation.
 *
 * Immediate safe-state behavior:
 *   Power relay OFF
 *   PWM = 0%
 *   PWM disabled
 *   Direction relays OFF
 *   Motor state STOPPED
 *   Emergency-stop latch ACTIVE
 *
 * The emergency-stop latch prevents subsequent motor starts
 * until motorControllerClearEmergencyStop() is called.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t motorControllerEmergencyStop(void);

/**
 * @brief Check whether the software emergency stop is active.
 *
 * @return true when emergency stop is latched.
 * @return false when emergency stop is not active.
 */
bool motorControllerIsEmergencyStopped(void);

/**
 * @brief Clear the software emergency-stop latch.
 *
 * Clearing the emergency-stop latch does NOT start the motor.
 * A separate motorControllerStart() request is required.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t motorControllerClearEmergencyStop(void);

/**
 * @brief Process the motor state machine.
 *
 * This function is non-blocking and should be called periodically.
 *
 * When emergency stop is active, the motor remains in the safe
 * stopped state regardless of normal state-machine processing.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t motorControllerProcess(void);

/**
 * @brief Get current motor state.
 *
 * @return Current motor state.
 */
MotorState_t motorControllerGetState(void);

/**
 * @brief Get current motor direction.
 *
 * @return Current motor direction.
 */
MotorDirection_t motorControllerGetDirection(void);

/**
 * @brief Get current commanded PWM duty.
 *
 * @return PWM duty percentage.
 */
uint32_t motorControllerGetDutyPercent(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_CONTROLLER_H */