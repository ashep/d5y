idf_component_register(
        SRCS cfg2.c
        INCLUDE_DIRS include
        REQUIRES esp_event nvs_flash dy_error dy_util
)
