idf_component_register(
    SRCS
        "app_main.c"

    INCLUDE_DIRS
        "."

    PRIV_INCLUDE_DIRS
        "../boards/orb_drive_main"

    REQUIRES
        core
        nfw_hal
        common
)