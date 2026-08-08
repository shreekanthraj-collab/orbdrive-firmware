/**
 * @file nfw_memory.c
 * @brief Platform memory abstraction implementation.
 */

#include "nfw_memory.h"

#include <stdlib.h>

/*===========================================================================
 * Public Functions
 *===========================================================================*/

void *nfwMalloc(size_t size)
{
    return malloc(size);
}

void nfwFree(void *ptr)
{
    free(ptr);
}
