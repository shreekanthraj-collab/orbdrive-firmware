/**
 * @file nfw_mutex.c
 * @brief ESP-IDF FreeRTOS implementation of the Orb Drive mutex abstraction.
 */

#include "nfw_mutex.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

NfwMutex_t nfwMutexCreate(void)
{
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();

    return (NfwMutex_t)mutex;
}

void nfwMutexDelete(NfwMutex_t mutex)
{
    if (mutex == NULL)
    {
        return;
    }

    vSemaphoreDelete((SemaphoreHandle_t)mutex);
}

bool nfwMutexLock(NfwMutex_t mutex)
{
    if (mutex == NULL)
    {
        return false;
    }

    return xSemaphoreTake(
        (SemaphoreHandle_t)mutex,
        portMAX_DELAY) == pdTRUE;
}

void nfwMutexUnlock(NfwMutex_t mutex)
{
    if (mutex == NULL)
    {
        return;
    }

    (void)xSemaphoreGive((SemaphoreHandle_t)mutex);
}