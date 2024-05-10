idf_component_register(
        SRCS "d5y_wifi.c"
        INCLUDE_DIRS "."
        REQUIRES "esp_wifi" "d5y_bt"
)
