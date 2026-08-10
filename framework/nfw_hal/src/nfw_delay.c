/**
 * @file nfw_delay.c
 * @brief ESP-IDF implementation of the Orb Drive delay HAL.
 */

#include "nfw_delay.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"

NfwStatus_t nfwDelayMs(uint32_t milliseconds)
{
    vTaskDelay(pdMS_TO_TICKS(milliseconds));

    return NFW_STATUS_OK;
}

NfwStatus_t nfwDelayUs(uint32_t microseconds)
{
    esp_rom_delay_us(microseconds);

    return NFW_STATUS_OK;
}
