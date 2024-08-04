idf_component_register(
        SRCS http.c json.c time.c weather.c
        INCLUDE_DIRS include
        REQUIRES esp_netif esp_http_client json dy_error
)
