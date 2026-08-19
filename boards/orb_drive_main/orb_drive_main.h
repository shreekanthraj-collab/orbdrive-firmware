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
 * INA226 current / voltage monitor
 * ========================================================================== */

#define ORB_INA226_I2C_ADDRESS          (0x40U)
#define ORB_INA226_RSHUNT_OHM           (0.010f)
#define ORB_INA226_MAX_CURRENT_A        (8.0f)

/* ============================================================================
 * I2C bus
 * ========================================================================== */

#define ORB_I2C_PORT                    (0U)
#define ORB_I2C_SDA_GPIO                (8U)
#define ORB_I2C_SCL_GPIO                (9U)
#define ORB_I2C_FREQUENCY_HZ            (400000U)

/* ============================================================================
 * Board identity
 * ========================================================================== */

#define ORB_BOARD_NAME                  "Orb Drive Main"
#define ORB_BOARD_VERSION               "1.0.0"

/* ============================================================================
 * LoRa SX1262 - Waveshare Core1262-HF
 *
 * Confirmed Orb Drive ESP32-S3 hardware mapping:
 *
 * GPIO11 -> SPI3 MOSI
 * GPIO12 -> SPI3 SCK
 * GPIO13 -> SPI3 MISO
 * GPIO14 -> SPI3 CS/NSS
 * GPIO5  -> LoRa DIO1 / interrupt
 * GPIO17 -> LoRa RXEN
 * GPIO18 -> LoRa TXEN
 * GPIO38 -> LoRa RESET
 * GPIO47 -> LoRa BUSY
 *
 * LoRa is the Node <-> Gateway transport layer.
 *
 * RS-485 remains dedicated to the flow sensors.
 * ========================================================================== */

#define ORB_LORA_SPI_MOSI_GPIO         (11U)
#define ORB_LORA_SPI_SCK_GPIO          (12U)
#define ORB_LORA_SPI_MISO_GPIO         (13U)
#define ORB_LORA_SPI_CS_GPIO           (14U)

#define ORB_LORA_DIO1_GPIO             (5U)
#define ORB_LORA_RXEN_GPIO             (17U)
#define ORB_LORA_TXEN_GPIO             (18U)
#define ORB_LORA_RESET_GPIO            (38U)
#define ORB_LORA_BUSY_GPIO             (47U)

/* ============================================================================
 * Motor / actuator GPIO ownership
 *
 * Confirmed Node ownership:
 *
 * GPIO39 -> Relay 3 -> Master relay
 * GPIO40 -> Relay 2 -> Bidirectional valve relay
 * GPIO41 -> Relay 1 -> Bidirectional valve relay
 * GPIO42 -> Soft-start PWM
 *
 * Relay 1 and Relay 2 form the bidirectional valve direction control.
 * Relay 3 is the master relay.
 * ========================================================================== */

#define ORB_GPIO_MASTER_RELAY          (39U)
#define ORB_GPIO_VALVE_RELAY_2         (40U)
#define ORB_GPIO_VALVE_RELAY_1         (41U)
#define ORB_GPIO_MOTOR_SOFT_START_PWM  (42U)

/*
 * Legacy motor-controller names.
 *
 * Keep these aliases so the existing motor-controller implementation
 * continues to compile without changing its control architecture.
 */
#define ORB_GPIO_MOTOR_POWER_RELAY     ORB_GPIO_MASTER_RELAY
#define ORB_GPIO_MOTOR_FORWARD_RELAY   ORB_GPIO_VALVE_RELAY_2
#define ORB_GPIO_MOTOR_REVERSE_RELAY   ORB_GPIO_VALVE_RELAY_1

/* ============================================================================
 * Relay electrical polarity
 *
 * Relay board is active LOW:
 *
 * LOW  = relay ON
 * HIGH = relay OFF
 * ========================================================================== */

#define ORB_RELAY_ON                   (false)
#define ORB_RELAY_OFF                  (true)

/* ============================================================================
 * Motor PWM
 * ========================================================================== */

#define ORB_MOTOR_PWM_FREQUENCY_HZ     (5000U)
#define ORB_MOTOR_PWM_RESOLUTION_BITS  (8U)

/*
 * Five commanded ramp points:
 *
 * 0 -> 20 -> 40 -> 60 -> 80 -> 100 %
 *
 * Total ramp target is 200 ms initially.
 * This is a validation parameter, not yet a mechanically certified value.
 */
#define ORB_MOTOR_RAMP_TOTAL_MS        (200U)
#define ORB_MOTOR_RAMP_STEP_MS         (40U)

/* ============================================================================
 * Motor PWM ramp points
 * ========================================================================== */

#define ORB_MOTOR_DUTY_0_PERCENT       (0U)
#define ORB_MOTOR_DUTY_20_PERCENT      (20U)
#define ORB_MOTOR_DUTY_40_PERCENT      (40U)
#define ORB_MOTOR_DUTY_60_PERCENT      (60U)
#define ORB_MOTOR_DUTY_80_PERCENT      (80U)
#define ORB_MOTOR_DUTY_100_PERCENT     (100U)

#ifdef __cplusplus
}
#endif

#endif /* ORB_DRIVE_MAIN_H */