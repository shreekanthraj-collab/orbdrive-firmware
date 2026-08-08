idf_component_register(
    SRCS
        "src/nfw_gpio.c"

    INCLUDE_DIRS
        "include"

    PRIV_INCLUDE_DIRS
        "private"

    REQUIRES
        driver

    PRIV_REQUIRES
        core
)
