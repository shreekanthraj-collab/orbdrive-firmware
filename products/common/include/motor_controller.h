/**
 * @file motor_controller.h
 * @brief Orb Drive motor/actuator control state machine.
 */

#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

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
 * @param direction Requested motor direction.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t motorControllerStart(MotorDirection_t direction);

/**
 * @brief Request motor stop.
 *
 * Sequence:
 *   Power relay OFF
 *   PWM ramp 100 -> 0%
 *   PWM OFF
 *   Direction OFF
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t motorControllerStop(void);

/**
 * @brief Process the motor state machine.
 *
 * This function is non-blocking and should be called periodically.
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