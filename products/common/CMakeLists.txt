idf_component_register(
    SRCS
        "src/condition_monitor.c"
        "src/fault_manager.c"
        "src/rs485.c"
        "src/motor_controller.c"

    INCLUDE_DIRS
        "include"

    PRIV_INCLUDE_DIRS
        "../../boards/orb_drive_main"

    REQUIRES
        core
        nfw_hal
)