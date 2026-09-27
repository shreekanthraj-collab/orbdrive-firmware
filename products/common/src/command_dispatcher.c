/**
 * @file command_dispatcher.c
 * @brief Gateway command to motor-controller dispatch.
 */

#include "command_dispatcher.h"

#include <stddef.h>

#include "motor_controller.h"

NfwStatus_t commandDispatcherDispatch(const GwCommandPacket_t *packet)
{
    if (packet == NULL)
    {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    switch (packet->command)
    {
        case GW_COMMAND_MOTOR_STOP:
            return motorControllerStop();

        case GW_COMMAND_EMERGENCY_STOP:
            return motorControllerEmergencyStop();

        case GW_COMMAND_EMERGENCY_RESET:
            return motorControllerClearEmergencyStop();

        default:
            return NFW_STATUS_NOT_SUPPORTED;
    }
}
