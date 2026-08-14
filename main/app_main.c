#include <stdio.h>

#include "motor_controller.h"

void app_main(void)
{
    NfwStatus_t status;

    printf("\n");
    printf("=====================================\n");
    printf(" Orb Drive Firmware\n");
    printf(" ESP32-S3\n");
    printf(" Motor Controller Initialized\n");
    printf("=====================================\n");

    status = motorControllerInit();

    if (status != NFW_STATUS_OK)
    {
        printf("Motor controller initialization FAILED: %d\n",
               (int)status);
        return;
    }

    printf("Motor controller initialization: PASS\n");
    printf("Motor state: STOPPED\n");
    printf("Motor PWM: 0%%\n");
}