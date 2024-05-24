#include <string.h>

#include "esp_err.h"
#include "esp_netif_types.h"
#include "esp_wifi.h"
#include "esp_log.h"

#include "dy/error.h"
#include "dy/cloud.h"

#define LTAG "DY_RTC"

static void update_tz_from_cloud() {
    dy_err_t err;
    dy_cloud_resp_time_t time;

    if (dy_nok(err = dy_cloud_get_time(&time))) {
        ESP_LOGE(LTAG, "dy_cloud_get_time: %s", dy_err_str(err));
        return;
    }

    if (strlen(time.tz_data) == 0) {
        ESP_LOGE(LTAG, "dy_cloud_get_time returned empty tz_data field");
        return;
    }

    setenv("TZ", time.tz_data, true);

    ESP_LOGI(LTAG, "timezone set: %s; %s", time.tz, time.tz_data);
}

dy_err_t dy_rtc_init() {
    esp_err_t esp_err;

    esp_err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &update_tz_from_cloud, NULL);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "esp_event_handler_register (IP) failed: %s", esp_err_to_name(esp_err));
    }

    return dy_ok();
}