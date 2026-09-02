idf_component_register(
    SRCS
    "src/as5600.c"
"src/valve_position.c"
        "src/condition_monitor.c"
        "src/fault_manager.c"
        "src/rs485.c"
        "src/motor_controller.c"
        "src/ina226.c"
        "src/eol_storage.c"
        "src/oc_config.c"
        "src/voltage_config.c"
        "src/lora_transport.c"
        "src/modbus_rtu.c"
        "src/flow_sensor.c"
        "src/gw_protocol.c"

    INCLUDE_DIRS
        "include"

    PRIV_INCLUDE_DIRS
        "../../boards/orb_drive_main"

    REQUIRES
        core
        nfw_hal
        nvs_flash
)