/**
 * @file nfw_version.c
 * @brief Orb Drive Framework version implementation.
 */

#include "nfw_version.h"
#include "nfw_types.h"

static const NfwVersion_t s_frameworkVersion =
{
    .major = NFW_VERSION_MAJOR,
    .minor = NFW_VERSION_MINOR,
    .patch = NFW_VERSION_PATCH
};

const NfwVersion_t *nfwVersionGet(void)
{
    return &s_frameworkVersion;
}

const char *nfwVersionString(void)
{
    return NFW_VERSION_STRING;
}
