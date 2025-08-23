idf_component_register(
        SRCS http_client.c location.c time.c weather.c fwupdate.c
        INCLUDE_DIRS include
        REQUIRES log esp_netif esp_http_client esp-tls json app_update esp_https_ota dy_error dy_appinfo dy_net_cfg
)
