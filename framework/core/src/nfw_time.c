/**
 * @file nfw_time.c
 * @brief Platform time abstraction implementation.
 */

#include "nfw_time.h"

#include <stdint.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <errno.h>
#include <unistd.h>
#endif

uint32_t nfwTimeNowMs(void)
{
#if defined(_WIN32) || defined(_WIN64)
    return (uint32_t)(GetTickCount64() % UINT32_MAX);
#else
    struct timespec ts;
#if defined(CLOCK_MONOTONIC)
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0U;
    }
#else
    if (timespec_get(&ts, TIME_UTC) == 0) {
        return 0U;
    }
#endif
    return (uint32_t)((uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL);
#endif
}

uint64_t nfwTimeNowUs(void)
{
#if defined(_WIN32) || defined(_WIN64)
    return (uint64_t)GetTickCount64() * 1000ULL;
#else
    struct timespec ts;
#if defined(CLOCK_MONOTONIC)
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0ULL;
    }
#else
    if (timespec_get(&ts, TIME_UTC) == 0) {
        return 0ULL;
    }
#endif
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
#endif
}
