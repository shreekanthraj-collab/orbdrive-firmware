#ifndef AS5600_H
#define AS5600_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "nfw_status.h"

#define AS5600_I2C_ADDRESS              (0x36U)
#define AS5600_REG_RAW_ANGLE            (0x0CU)

typedef struct
{
    uint8_t i2cAddress;
} As5600Config_t;

NfwStatus_t as5600Init(
    const As5600Config_t *config);

NfwStatus_t as5600ReadRawAngle(
    uint16_t *rawAngle);

NfwStatus_t as5600ReadAngleDegrees(
    float *angleDegrees);

bool as5600IsInitialized(void);

#ifdef __cplusplus
}
#endif

#endif /* AS5600_H */
