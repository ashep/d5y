idf_component_register(
        SRCS wifi.c ip.c bt_cfg.c
        INCLUDE_DIRS include
        REQUIRES esp_wifi dy_error dy_bt
)
