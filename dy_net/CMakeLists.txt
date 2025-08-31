idf_component_register(
        SRCS net.c
        INCLUDE_DIRS include
        REQUIRES esp_wifi dy_error dy_cfg2 dy_internal dy_bt
)
