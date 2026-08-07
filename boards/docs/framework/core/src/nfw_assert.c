/**
 * @file nfw_assert.c
 * @brief Orb Drive Framework assertion support.
 */

#include "nfw_assert.h"

void nfwAssertFailure(const char *expr,
                      const char *file,
                      uint32_t line)
{
    /* Parameters intentionally kept for debugger inspection. */
    (void)expr;
    (void)file;
    (void)line;

    /*
     * Default framework behavior:
     * Stay here so a debugger can inspect the failure.
     *
     * Platform-specific implementations may later replace this
     * with logging, watchdog reset, reboot, or fault recording.
     */
    for (;;)
    {
    }
}
