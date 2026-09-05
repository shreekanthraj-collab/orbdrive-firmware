idf_component_register(
    SRCS
        "app_main.c"
        "ota_wifi_ap.c"
        "ota_server.c"

    INCLUDE_DIRS
        "."

    PRIV_INCLUDE_DIRS
        "../boards/orb_drive_main"

    REQUIRES
        core
        nfw_hal
        common
        app_update
        esp_wifi
        esp_netif
        esp_event
        esp_http_server
)
