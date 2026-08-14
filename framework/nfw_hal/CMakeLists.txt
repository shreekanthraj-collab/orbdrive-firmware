idf_component_register(
    SRCS
        "src/nfw_gpio.c"
        "src/nfw_delay.c"
        "src/nfw_uart.c"
        "src/nfw_pwm.c"

    INCLUDE_DIRS
        "include"

    PRIV_INCLUDE_DIRS
        "private"

    REQUIRES
        core
        driver
        freertos
)