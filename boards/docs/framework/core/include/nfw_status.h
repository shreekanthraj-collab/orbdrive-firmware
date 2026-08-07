/**
 * @file nfw_status.h
 * @brief Common status codes used throughout the Orb Drive Framework.
 */

#ifndef NFW_STATUS_H
#define NFW_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    NFW_STATUS_OK = 0,

    NFW_STATUS_ERROR,

    NFW_STATUS_INVALID_ARGUMENT,

    NFW_STATUS_INVALID_STATE,

    NFW_STATUS_TIMEOUT,

    NFW_STATUS_NOT_SUPPORTED,

    NFW_STATUS_NO_MEMORY,

    NFW_STATUS_BUSY,

    NFW_STATUS_NOT_FOUND,

    NFW_STATUS_ALREADY_EXISTS

} NfwStatus_t;

#ifdef __cplusplus
}
#endif

#endif /* NFW_STATUS_H */
