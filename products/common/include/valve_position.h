#ifndef VALVE_POSITION_H
#define VALVE_POSITION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "nfw_status.h"

typedef struct
{
    uint16_t closedAngleRaw;
    uint16_t openAngleRaw;
} ValvePositionConfig_t;

NfwStatus_t valvePositionInit(
    const ValvePositionConfig_t *config);

NfwStatus_t valvePositionGetOpeningPercent(
    uint8_t *openingPercent);

NfwStatus_t valvePositionGetRawAngle(
    uint16_t *rawAngle);

#ifdef __cplusplus
}
#endif

#endif /* VALVE_POSITION_H */
