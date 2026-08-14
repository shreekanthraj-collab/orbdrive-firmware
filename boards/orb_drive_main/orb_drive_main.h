/**
 * @file orb_drive_main.h
 * @brief Orb Drive Main Board hardware definition.
 */

#ifndef ORB_DRIVE_MAIN_H
#define ORB_DRIVE_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * Board identity
 * ========================================================================== */

#define ORB_BOARD_NAME    "Orb Drive Main"
#define ORB_BOARD_VERSION "1.0.0"

/* ============================================================================
 * Motor / actuator GPIO ownership
 *
 * Confirmed Node ownership:
 * GPIO39 -> Power relay
 * GPIO40 -> Forward relay
 * GPIO41 -> Reverse relay
 * GPIO42 -> Soft-start PWM
 * ========================================================================== */

#define ORB_GPIO_MOTOR_POWER_RELAY      (39U)
#define ORB_GPIO_MOTOR_FORWARD_RELAY    (40U)
#define ORB_GPIO_MOTOR_REVERSE_RELAY    (41U)
#define ORB_GPIO_MOTOR_SOFT_START_PWM   (42U)

/* ============================================================================
 * Relay electrical polarity
 *
 * Relay board is active LOW:
 * LOW  = relay ON
 * HIGH = relay OFF
 * ========================================================================== */

#define ORB_RELAY_ON                    (false)
#define ORB_RELAY_OFF                   (true)

/* ============================================================================
 * Motor PWM
 * ========================================================================== */

#define ORB_MOTOR_PWM_FREQUENCY_HZ      (5000U)
#define ORB_MOTOR_PWM_RESOLUTION_BITS   (8U)

/*
 * Five commanded ramp points:
 *
 * 0 -> 20 -> 40 -> 60 -> 80 -> 100 %
 *
 * Total ramp target is 200 ms initially.
 * This is a validation parameter, not yet a mechanically certified value.
 */
#define ORB_MOTOR_RAMP_TOTAL_MS         (200U)
#define ORB_MOTOR_RAMP_STEP_MS          (40U)

/* ============================================================================
 * Motor PWM ramp points
 * ========================================================================== */

#define ORB_MOTOR_DUTY_0_PERCENT        (0U)
#define ORB_MOTOR_DUTY_20_PERCENT       (20U)
#define ORB_MOTOR_DUTY_40_PERCENT       (40U)
#define ORB_MOTOR_DUTY_60_PERCENT       (60U)
#define ORB_MOTOR_DUTY_80_PERCENT       (80U)
#define ORB_MOTOR_DUTY_100_PERCENT      (100U)

#ifdef __cplusplus
}
#endif

#endif /* ORB_DRIVE_MAIN_H */