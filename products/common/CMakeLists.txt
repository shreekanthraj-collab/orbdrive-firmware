idf_component_register(
    SRCS
        "src/condition_monitor.c"
        "src/fault_manager.c"
        "src/rs485.c"

    INCLUDE_DIRS
        "include"

    REQUIRES
        core

    PRIV_REQUIRES
        nfw_hal
)