idf_component_register(
    SRCS
        "src/nfw_gpio.c"
        "src/nfw_delay.c"
        "src/nfw_uart.c"

    INCLUDE_DIRS
        "include"

    PRIV_INCLUDE_DIRS
        "private"

    REQUIRES
        driver
)