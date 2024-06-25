idf_component_register(
        SRCS dy_net_cfg.c
        INCLUDE_DIRS include
        REQUIRES esp_wifi dy_error dy_net dy_bt
)
