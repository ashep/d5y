idf_component_register(
        SRCS "cronus_wifi.c"
        INCLUDE_DIRS "."
        REQUIRES "esp_wifi" "cronus_bt"
)
