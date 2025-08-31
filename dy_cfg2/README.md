# dy_cfg2

Persistent configuration storage & in‑RAM cache backed by ESP-IDF NVS.

Successor to (and replacement for) [dy_cfg](../dy_cfg/README.md).

## Features

- Typed getters & setters: `uint8_t`, `float`, `string`.
- Optional default getters (`*_dft`) that return provided default when key absent.
- In‑memory cache to avoid repeated NVS reads (populated lazily on first read or after write).
- Change notification via ESP Event loop (`DY_CFG2_EVENT_BASE`, event id `DY_CFG2_EVENT_SET`).
- Compact NVS key format (4 hex digits) derived from integer IDs you define.

## When to use

Use `dy_cfg2` for small device/user settings that must survive reboot / OTA. Avoid storing large blobs or frequently
mutating high‑churn values (consider RAM only or a different partition for that).

## Initialization

Call after `nvs_flash_init()` and after creating the default event loop (if you plan to subscribe to events):

```c
#include "nvs_flash.h"
#include "esp_event.h"
#include "dy/cfg2.h"

void app_init_cfg() {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // if not already created
    dy_err_t err;
    if (dy_is_err(err = dy_cfg2_init())) {
        ESP_LOGE("APP", "cfg2 init failed: %s", dy_err_str(err));
    }
}
```

## Defining IDs

You choose integer IDs for each setting. Keep IDs in range `0x0000`–`0xEFFF` inclusive. IDs `0xF000-0xFFFF` are reserved
for internal purposes.

## API

Common:

- `dy_err_t dy_cfg2_init()`. Initialize the module.
- `bool dy_cfg2_is_set(int id)`. Check if the value is set.

Unsigned 8‑bit values:

- `dy_err_t dy_cfg2_get_u8(int id, uint8_t *dst);`
- `dy_err_t dy_cfg2_get_u8_dft(int id, uint8_t *dst, uint8_t dft);`
- `dy_err_t dy_cfg2_set_u8(int id, uint8_t val);`

Float values:

- `dy_err_t dy_cfg2_get_float(int id, float *dst);`
- `dy_err_t dy_cfg2_get_float_dft(int id, float *dst, float dft);`
- `dy_err_t dy_cfg2_set_float(int id, float val);`

String values (max length is `DY_CFG2_STR_MAX_LEN`, incl. terminator):

- `dy_err_t dy_cfg2_get_str(int id, char *dst);`
- `dy_err_t dy_cfg2_get_str_dft(int id, char *dst, const char *dft);`
- `dy_err_t dy_cfg2_set_str(int id, const char *val);`

### Return Codes (dy_err_t->code)

- `DY_ERR_OK` success
- `DY_ERR_INVALID_ARG` null pointer / type mismatch / string too long
- `DY_ERR_NOT_CONFIGURED` `dy_cfg2_init()` not yet called
- `DY_ERR_NOT_FOUND` key absent (only an error for non‑`*_dft` getters)
- `DY_ERR_NO_MEM` allocation failure (cache)
- `DY_ERR_FAILED` NVS failure wrapper

Use `dy_is_err(err)` and `dy_err_str(err)` helpers (see `dy_error`).

## Usage Examples

### Set & Get with Default

```c
uint8_t brightness;
if (dy_is_err(dy_cfg2_get_u8_dft(CFG_ID_BRIGHTNESS, &brightness, 50))) {
    // only unexpected errors arrive here
}
// Adjust & persist
brightness = 80;
dy_cfg2_set_u8(CFG_ID_BRIGHTNESS, brightness);
```

### Float Calibration Value

```c
float temp_offset;
dy_cfg2_get_float_dft(CFG_ID_TEMP_OFFSET, &temp_offset, 0.0f);
```

### Strings

```c
char ssid[DY_CFG2_STR_MAX_LEN];
if (dy_is_err(dy_cfg2_get_str_dft(CFG_ID_WIFI_SSID, ssid, ""))) {
    // handle error
}
if (strcmp(ssid, new_ssid) != 0) {
    if (dy_is_err(dy_cfg2_set_str(CFG_ID_WIFI_SSID, new_ssid))) {
        // handle error
    }
}
```

## Change Events

Whenever a value is successfully written, an event is posted:

- Event base: `DY_CFG2_EVENT_BASE`.
- Event id: `DY_CFG2_EVENT_SET`.
- Event data: `dy_cfg2_evt_set_t { int id; uint8_t type; void *val; }`

Register a handler:

```c
static void on_cfg_set(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    (void)handler_arg; (void)base; (void)id;
    dy_cfg2_evt_set_t *e = event_data;
    switch (e->type) {
        case DY_CFG2_VALUE_TYPE_U8:
            ESP_LOGI("CFG", "u8 id=%04x val=%u", e->id, *(uint8_t*)e->val); break;
        case DY_CFG2_VALUE_TYPE_FLOAT:
            ESP_LOGI("CFG", "float id=%04x val=%f", e->id, *(float*)e->val); break;
        case DY_CFG2_VALUE_TYPE_STR:
            ESP_LOGI("CFG", "str id=%04x val=%s", e->id, (char*)e->val); break;
    }
}

void subscribe_cfg_events() {
    ESP_ERROR_CHECK(esp_event_handler_register(DY_CFG2_EVENT_BASE, DY_CFG2_EVENT_SET, on_cfg_set, NULL));
}
```

`val` points to the value passed into the setter call (not a copy); copy it if you need persistence beyond the handler
scope.

## Caching Behavior

- Reads first consult in‑RAM hash. If absent, the value is loaded from NVS and cached.
- Writes update NVS, commit, update cache, then emit event.
- No eviction; all distinct keys read or written remain cached until reboot.

## Constraints & Notes

- ID Range: stick to `0x0000`–`0xEFFF`. IDs `0xF000-0xFFFF` are reserved for internal purposes.
- Strings: ensure length < `DY_CFG2_STR_MAX_LEN` (including terminating null) or `DY_ERR_INVALID_ARG` is returned.
- Concurrency: no internal locking; if used from multiple tasks concurrently, serialize external access (e.g., mutex) if
  simultaneous writes possible.
- Performance: each write commits immediately (`nvs_commit`). Batch many writes manually if you need speed (future
  enhancement could add delayed commit).
- Flash Wear: because every set commits, avoid high‑frequency updates (e.g., counters) or debounce them.

## Future Ideas

- Additional types (int32, bool alias, blobs)
- Iterator over existing keys
