# =============================================================================
# Orb Drive Firmware Framework
# Framework Core Component
# =============================================================================

idf_component_register(
    SRCS
        "src/nfw_core.c"
        "src/nfw_assert.c"
        "src/nfw_version.c"

    INCLUDE_DIRS
        "include"

    PRIV_INCLUDE_DIRS
        "private"
)
