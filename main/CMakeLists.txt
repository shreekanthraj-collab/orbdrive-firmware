idf_component_register(
    SRCS
        "app_main.c"

    INCLUDE_DIRS
        "."

    PRIV_INCLUDE_DIRS
        "../boards/orb_drive_main"

    # Required components
    # Add/remove dependencies here when needed.
    REQUIRES
        core
        nfw_hal
        common
        app_update
)
