/**
 * @file nfw_memory.c
 * @brief ESP-IDF implementation of the Orb Drive memory abstraction.
 */

#include "nfw_memory.h"

#include <stdlib.h>

void *nfwMalloc(size_t size)
{
    if (size == 0U)
    {
        return NULL;
    }

    return malloc(size);
}

void nfwFree(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    free(ptr);
}