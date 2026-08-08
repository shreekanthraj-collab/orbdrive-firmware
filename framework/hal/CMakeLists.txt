idf_component_register(
    SRCS
        "src/nfw_gpio.c"
        "src/nfw_delay.c"

    INCLUDE_DIRS
        "include"

    PRIV_INCLUDE_DIRS
        "private"

    REQUIRES
        driver
)