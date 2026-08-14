/**
 * @file nfw_pwm.h
 * @brief Orb Drive PWM Hardware Abstraction Layer.
 */

#ifndef NFW_PWM_H
#define NFW_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "nfw_status.h"

/**
 * @brief Initialize the PWM peripheral.
 *
 * The PWM output is initialized to 0% duty.
 *
 * @param pin GPIO used for PWM output.
 * @param frequency_hz PWM frequency.
 * @param resolution_bits Requested duty command resolution.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwPwmInit(
    uint32_t pin,
    uint32_t frequency_hz,
    uint32_t resolution_bits);

/**
 * @brief Set PWM duty cycle in percent.
 *
 * @param duty_percent Duty cycle from 0 to 100.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwPwmSetDutyPercent(uint32_t duty_percent);

/**
 * @brief Get the currently commanded PWM duty cycle.
 *
 * @param duty_percent Pointer receiving duty percentage.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwPwmGetDutyPercent(uint32_t *duty_percent);

/**
 * @brief Disable the PWM output and force it LOW.
 *
 * @return NFW_STATUS_OK on success.
 */
NfwStatus_t nfwPwmDisable(void);

#ifdef __cplusplus
}
#endif

#endif /* NFW_PWM_H */