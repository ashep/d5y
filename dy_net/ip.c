#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif_types.h"

#include "dy/error.h"
#include "dy/_net.h"

static void on_got_addr(ip_event_got_ip_t *ev) {
    ESP_LOGI(LTAG, "IP_EVENT_STA_GOT_IP: addr="IPSTR"; mask="IPSTR"; gw="IPSTR,
             IP2STR(&ev->ip_info.ip), IP2STR(&ev->ip_info.netmask), IP2STR(&ev->ip_info.gw));
}

static void on_lost_addr() {
    ESP_LOGW(LTAG, "IP_EVENT_STA_LOST_IP");
    ESP_LOGI(LTAG, "trying to reconnect");

    esp_err_t err;

    if ((err = esp_wifi_disconnect()) != ESP_OK) {
        ESP_LOGE(LTAG, "%s: esp_wifi_disconnect failed: %s", __func__, esp_err_to_name(err));
    }
}

void ip_ev_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data) {
    switch (event_id) {
        case IP_EVENT_STA_GOT_IP:
            on_got_addr((ip_event_got_ip_t *) data);
            break;

        case IP_EVENT_STA_LOST_IP:
            on_lost_addr();
            break;

        default:
            ESP_LOGI(LTAG, "IP_EVENT: %ld", event_id);
            break;
    }
}