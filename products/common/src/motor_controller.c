/**
 * @file motor_controller.c
 * @brief Orb Drive motor/actuator control state machine.
 *
 * Frozen motor sequence:
 *
 * START:
 *   Direction relay ON
 *        ->
 *   PWM = 0%
 *        ->
 *   PWM ramp 0 -> 20 -> 40 -> 60 -> 80 -> 100%
 *        ->
 *   Power relay ON
 *        ->
 *   RUNNING
 *
 * STOP:
 *   Power relay OFF
 *        ->
 *   PWM ramp 100 -> 80 -> 60 -> 40 -> 20 -> 0%
 *        ->
 *   PWM OFF
 *        ->
 *   Direction relay OFF
 *        ->
 *   STOPPED
 *
 * The ramp is non-blocking. motorControllerProcess() advances
 * the state machine using nfwTimeNowMs().
 */

#include "motor_controller.h"

#include <stdbool.h>
#include <stdint.h>

#include "nfw_gpio.h"
#include "nfw_pwm.h"
#include "nfw_time.h"

#include "orb_drive_main.h"

/* ============================================================================
 * Internal configuration
 * ========================================================================== */

#define MOTOR_RAMP_STEP_COUNT       (5U)

/*
 * Frozen ramp:
 *
 * 0 -> 20 -> 40 -> 60 -> 80 -> 100%
 *
 * 40 ms between commanded ramp points.
 * Total ramp time = 200 ms.
 */
static const uint32_t s_ramp_duty_percent[MOTOR_RAMP_STEP_COUNT + 1U] =
{
    ORB_MOTOR_DUTY_0_PERCENT,
    ORB_MOTOR_DUTY_20_PERCENT,
    ORB_MOTOR_DUTY_40_PERCENT,
    ORB_MOTOR_DUTY_60_PERCENT,
    ORB_MOTOR_DUTY_80_PERCENT,
    ORB_MOTOR_DUTY_100_PERCENT
};

/* ============================================================================
 * Internal state
 * ========================================================================== */

static bool s_initialized = false;

static MotorState_t s_state = MOTOR_STATE_STOPPED;

static MotorDirection_t s_direction = MOTOR_DIRECTION_NONE;

static uint32_t s_duty_percent = ORB_MOTOR_DUTY_0_PERCENT;

static uint32_t s_ramp_index = 0U;

static uint32_t s_last_ramp_time_ms = 0U;

/* ============================================================================
 * Internal helpers
 * ========================================================================== */

static NfwStatus_t motorSetDirection(MotorDirection_t direction)
{
    NfwStatus_t status;

    /*
     * Both direction relays are first forced OFF.
     * This guarantees that forward and reverse cannot be active together.
     */
    status = nfwGpioWrite(
        ORB_GPIO_MOTOR_FORWARD_RELAY,
        ORB_RELAY_OFF);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = nfwGpioWrite(
        ORB_GPIO_MOTOR_REVERSE_RELAY,
        ORB_RELAY_OFF);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    if (direction == MOTOR_DIRECTION_FORWARD)
    {
        return nfwGpioWrite(
            ORB_GPIO_MOTOR_FORWARD_RELAY,
            ORB_RELAY_ON);
    }

    if (direction == MOTOR_DIRECTION_REVERSE)
    {
        return nfwGpioWrite(
            ORB_GPIO_MOTOR_REVERSE_RELAY,
            ORB_RELAY_ON);
    }

    return NFW_STATUS_OK;
}

static NfwStatus_t motorDirectionOff(void)
{
    NfwStatus_t status;

    status = nfwGpioWrite(
        ORB_GPIO_MOTOR_FORWARD_RELAY,
        ORB_RELAY_OFF);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    return nfwGpioWrite(
        ORB_GPIO_MOTOR_REVERSE_RELAY,
        ORB_RELAY_OFF);
}

static NfwStatus_t motorPowerOff(void)
{
    return nfwGpioWrite(
        ORB_GPIO_MOTOR_POWER_RELAY,
        ORB_RELAY_OFF);
}

static NfwStatus_t motorPowerOn(void)
{
    return nfwGpioWrite(
        ORB_GPIO_MOTOR_POWER_RELAY,
        ORB_RELAY_ON);
}

static NfwStatus_t motorSetDuty(uint32_t duty_percent)
{
    NfwStatus_t status;

    status = nfwPwmSetDutyPercent(duty_percent);

    if (status == NFW_STATUS_OK)
    {
        s_duty_percent = duty_percent;
    }

    return status;
}

/* ============================================================================
 * Public API
 * ========================================================================== */

NfwStatus_t motorControllerInit(void)
{
    NfwStatus_t status;

    if (s_initialized)
    {
        return NFW_STATUS_ALREADY_INITIALIZED;
    }

    /*
     * Safe hardware state:
     *
     * Power relay    OFF
     * Forward relay  OFF
     * Reverse relay  OFF
     * PWM            0%
     */
    NfwGpioConfig_t power_config =
    {
        .pin = ORB_GPIO_MOTOR_POWER_RELAY,
        .direction = NFW_GPIO_DIRECTION_OUTPUT,
        .pull = NFW_GPIO_PULL_NONE,
        .initialLevel = ORB_RELAY_OFF
    };

    NfwGpioConfig_t forward_config =
    {
        .pin = ORB_GPIO_MOTOR_FORWARD_RELAY,
        .direction = NFW_GPIO_DIRECTION_OUTPUT,
        .pull = NFW_GPIO_PULL_NONE,
        .initialLevel = ORB_RELAY_OFF
    };

    NfwGpioConfig_t reverse_config =
    {
        .pin = ORB_GPIO_MOTOR_REVERSE_RELAY,
        .direction = NFW_GPIO_DIRECTION_OUTPUT,
        .pull = NFW_GPIO_PULL_NONE,
        .initialLevel = ORB_RELAY_OFF
    };

    status = nfwGpioInit(&power_config);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = nfwGpioInit(&forward_config);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = nfwGpioInit(&reverse_config);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = nfwPwmInit(
        ORB_GPIO_MOTOR_SOFT_START_PWM,
        ORB_MOTOR_PWM_FREQUENCY_HZ,
        ORB_MOTOR_PWM_RESOLUTION_BITS);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = nfwPwmSetDutyPercent(
        ORB_MOTOR_DUTY_0_PERCENT);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = motorPowerOff();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    status = motorDirectionOff();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    s_state = MOTOR_STATE_STOPPED;
    s_direction = MOTOR_DIRECTION_NONE;
    s_duty_percent = ORB_MOTOR_DUTY_0_PERCENT;
    s_ramp_index = 0U;
    s_last_ramp_time_ms = 0U;
    s_initialized = true;

    return NFW_STATUS_OK;
}

NfwStatus_t motorControllerStart(MotorDirection_t direction)
{
    NfwStatus_t status;

    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    if (direction != MOTOR_DIRECTION_FORWARD &&
        direction != MOTOR_DIRECTION_REVERSE)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (s_state != MOTOR_STATE_STOPPED)
    {
        return NFW_STATUS_BUSY;
    }

    /*
     * Safety condition:
     * Power relay must be OFF before a new start sequence begins.
     */
    status = motorPowerOff();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    /*
     * PWM starts from zero.
     */
    status = motorSetDuty(
        ORB_MOTOR_DUTY_0_PERCENT);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    /*
     * Direction relay is enabled BEFORE PWM ramp.
     */
    status = motorSetDirection(direction);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    s_direction = direction;
    s_ramp_index = 0U;
    s_last_ramp_time_ms = nfwTimeNowMs();
    s_state = MOTOR_STATE_RAMPING_UP;

    return NFW_STATUS_OK;
}

NfwStatus_t motorControllerStop(void)
{
    NfwStatus_t status;

    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    if (s_state == MOTOR_STATE_STOPPED)
    {
        return NFW_STATUS_OK;
    }

    /*
     * STOP sequence begins by removing motor power.
     *
     * PWM remains active while the ramp-down is performed.
     */
    status = motorPowerOff();

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    /*
     * If the motor is already at zero, complete the stop immediately.
     */
    if (s_duty_percent == ORB_MOTOR_DUTY_0_PERCENT)
    {
        status = nfwPwmDisable();

        if (status != NFW_STATUS_OK)
        {
            return status;
        }

        status = motorDirectionOff();

        if (status != NFW_STATUS_OK)
        {
            return status;
        }

        s_direction = MOTOR_DIRECTION_NONE;
        s_state = MOTOR_STATE_STOPPED;
        s_ramp_index = 0U;

        return NFW_STATUS_OK;
    }

    /*
     * Find the current ramp point.
     */
    if (s_duty_percent >= ORB_MOTOR_DUTY_100_PERCENT)
    {
        s_ramp_index = MOTOR_RAMP_STEP_COUNT;
    }
    else if (s_duty_percent >= ORB_MOTOR_DUTY_80_PERCENT)
    {
        s_ramp_index = 4U;
    }
    else if (s_duty_percent >= ORB_MOTOR_DUTY_60_PERCENT)
    {
        s_ramp_index = 3U;
    }
    else if (s_duty_percent >= ORB_MOTOR_DUTY_40_PERCENT)
    {
        s_ramp_index = 2U;
    }
    else if (s_duty_percent >= ORB_MOTOR_DUTY_20_PERCENT)
    {
        s_ramp_index = 1U;
    }
    else
    {
        s_ramp_index = 0U;
    }

    s_last_ramp_time_ms = nfwTimeNowMs();
    s_state = MOTOR_STATE_RAMPING_DOWN;

    return NFW_STATUS_OK;
}

NfwStatus_t motorControllerProcess(void)
{
    uint32_t now_ms;
    uint32_t elapsed_ms;
    NfwStatus_t status;

    if (!s_initialized)
    {
        return NFW_STATUS_INVALID_STATE;
    }

    if (s_state != MOTOR_STATE_RAMPING_UP &&
        s_state != MOTOR_STATE_RAMPING_DOWN)
    {
        return NFW_STATUS_OK;
    }

    now_ms = nfwTimeNowMs();
    elapsed_ms = now_ms - s_last_ramp_time_ms;

    if (elapsed_ms < ORB_MOTOR_RAMP_STEP_MS)
    {
        return NFW_STATUS_OK;
    }

    s_last_ramp_time_ms = now_ms;

    if (s_state == MOTOR_STATE_RAMPING_UP)
    {
        /*
         * Advance:
         * 0 -> 20 -> 40 -> 60 -> 80 -> 100
         */
        if (s_ramp_index < MOTOR_RAMP_STEP_COUNT)
        {
            s_ramp_index++;
        }

        status = motorSetDuty(
            s_ramp_duty_percent[s_ramp_index]);

        if (status != NFW_STATUS_OK)
        {
            return status;
        }

        /*
         * Power is enabled ONLY after PWM reaches 100%.
         */
        if (s_ramp_index >= MOTOR_RAMP_STEP_COUNT)
        {
            status = motorPowerOn();

            if (status != NFW_STATUS_OK)
            {
                return status;
            }

            s_state = MOTOR_STATE_RUNNING;
        }

        return NFW_STATUS_OK;
    }

    /*
     * RAMPING_DOWN
     *
     * 100 -> 80 -> 60 -> 40 -> 20 -> 0
     */
    if (s_ramp_index > 0U)
    {
        s_ramp_index--;
    }

    status = motorSetDuty(
        s_ramp_duty_percent[s_ramp_index]);

    if (status != NFW_STATUS_OK)
    {
        return status;
    }

    /*
     * Once PWM reaches zero:
     *   PWM OFF
     *   Direction OFF
     *   STOPPED
     */
    if (s_ramp_index == 0U)
    {
        status = nfwPwmDisable();

        if (status != NFW_STATUS_OK)
        {
            return status;
        }

        status = motorDirectionOff();

        if (status != NFW_STATUS_OK)
        {
            return status;
        }

        s_direction = MOTOR_DIRECTION_NONE;
        s_state = MOTOR_STATE_STOPPED;
    }

    return NFW_STATUS_OK;
}

MotorState_t motorControllerGetState(void)
{
    return s_state;
}

MotorDirection_t motorControllerGetDirection(void)
{
    return s_direction;
}

uint32_t motorControllerGetDutyPercent(void)
{
    return s_duty_percent;
}