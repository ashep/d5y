idf_component_register(
        SRCS rtc.c
        INCLUDE_DIRS include
        REQUIRES esp_wifi dy_error dy_cloud
)
