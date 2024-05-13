idf_component_register(
        SRCS dy_wifi.c
        INCLUDE_DIRS include
        REQUIRES esp_wifi dy_error dy_bt
)
