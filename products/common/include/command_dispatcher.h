/**
 * @file command_dispatcher.h
 * @brief Gateway command to motor-controller dispatch.
 */

#ifndef COMMAND_DISPATCHER_H
#define COMMAND_DISPATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gw_protocol.h"
#include "nfw_status.h"

/**
 * @brief Dispatch a parsed gateway command to the motor controller.
 *
 * This layer deliberately handles only commands whose motor-controller
 * semantics are already defined. Command-specific argument contracts that
 * are not yet defined are left for their own integration work.
 *
 * @param packet Parsed gateway command packet.
 *
 * @return NFW_STATUS_OK when the command was accepted.
 * @return NFW_STATUS_INVALID_ARGUMENT when packet is invalid.
 * @return NFW_STATUS_NOT_SUPPORTED when the command is not yet wired.
 * @return Motor-controller status for supported commands.
 */
NfwStatus_t commandDispatcherDispatch(const GwCommandPacket_t *packet);

#ifdef __cplusplus
}
#endif

#endif /* COMMAND_DISPATCHER_H */
