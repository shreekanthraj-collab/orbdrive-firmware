# =============================================================================
# Orb Drive Firmware Framework
# Framework Platform Component
# =============================================================================

idf_component_register(
    SRCS
        "src/nfw_memory.c"
        "src/nfw_mutex.c"

    INCLUDE_DIRS
        "include"

    PRIV_INCLUDE_DIRS
        "private"

    REQUIRES
        core
        freertos
)