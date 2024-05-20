idf_component_register(
        SRCS cloud.c
        INCLUDE_DIRS include
        REQUIRES esp_netif esp_http_client json dy_error
)
