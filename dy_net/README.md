# dy_net

`dy_net` is a thin helper around ESP-IDF Wi‑Fi Station (STA) bringing:

- One‑call Wi‑Fi station initialization.
- Persisted "network configured" flag.
- Helper to set / clear Wi‑Fi credentials at runtime.
- Automatic reconnect watchdog while a configuration exists.

It deliberately keeps the public API minimal; higher‑level UX (Bluetooth provisioning, scanning, status queries) lives
in [`dy_net_cfg`](../dy_net_cfg/README.md).

## When to use

Use `dy_net` if your application only needs to:

- Initialize Wi‑Fi once.
- Apply credentials obtained elsewhere (e.g. from provisioning, hard‑coded, or received over another channel).
- Rely on automatic reconnect logic, without implementing your own retry loop.

If you need Bluetooth provisioning flows or to obtain current connection state / scanned SSIDs over BT, add `dy_net_cfg`
on top.

## API

### dy_err_t dy_net_init()

Initializes ESP networking stack in **station mode** only:

1. Calls `esp_netif_init()`.
2. Creates default STA netif (`esp_netif_create_default_wifi_sta`).
3. Calls `esp_wifi_init()` with default config.
4. Registers IP + Wi‑Fi event handlers (internal).
5. Sets Wi‑Fi mode to `WIFI_MODE_STA` and starts Wi‑Fi driver.
6. Performs an initial `esp_wifi_connect()` attempt (it will fail harmlessly if no credentials configured yet).
7. Loads persisted "configured" flag (see Persistence below).
8. Spawns a watchdog FreeRTOS task that re‑issues `esp_wifi_connect()` every 10s while credentials are configured but
   the station is disconnected.

Returns `dy_ok()` on success, otherwise a `dy_err_t` with `DY_ERR_FAILED` category and a human readable message.

Call this once early in your startup after system / NVS / event loop init (follow normal ESP‑IDF ordering used elsewhere
in the project). It is safe to call `dy_net_set_config_and_connect` only after this succeeds.

### void dy_net_set_config_and_connect(const char *ssid, const char *password)

Applies new STA credentials and immediately calls `esp_wifi_connect()`.

Behavior notes:

- SSID (max 32 bytes) and password (max 64 bytes) are truncated if longer.
- On first successful connection (WIFI_EVENT_STA_CONNECTED) a persistent flag is written marking the network as
  configured.
- Does not block waiting for IP acquisition; subscribe to ESP‑IDF events if you need notification (e.g.
  `IP_EVENT_STA_GOT_IP`).

### void dy_net_clear_config_and_disconnect()

Clears stored credentials, resets the persistent "configured" flag, and disconnects STA.

After calling this, the watchdog stops reconnect attempts until new credentials are applied.

## Typical lifecycle

```c
// 1. Initialize dependent subsystems (logging, NVS, event loop) elsewhere.
dy_err_t err;
if (dy_is_err(err = dy_net_init())) {
    ESP_LOGE("APP", "dy_net_init failed: %s", dy_err_str(err));
}

// 2. (Optional) Provide credentials — from provisioning or configuration.
dy_net_set_config_and_connect("MyWiFi", "SuperSecret");

// 3. Listen to events for IP acquisition.
//    - IP_EVENT_STA_GOT_IP => ready for network traffic.

// ... later, to forget credentials:
dy_net_clear_config_and_disconnect();
```

## Error handling

Only `dy_net_init` returns a `dy_err_t`; other functions log ESP‑IDF errors internally. Hook into ESP‑IDF wifi/ip events
if you need richer failure reporting (e.g. reason codes on disconnect).

## Thread safety

All public functions must be called from a context where the ESP‑IDF Wi‑Fi API is safe to use (typically the main task
or another task after `esp_wifi_init`). No internal locking is implemented beyond what ESP‑IDF provides.

## Limitations / Notes

- STA mode only; AP / AP+STA not supported.
- No direct API to query current status (use events or `dy_net_cfg`).
- No credential validation before attempting connect; relies on ESP‑IDF failure callbacks.
- The watchdog uses a fixed 10s retry interval (no exponential backoff).

## Interoperability with dy_net_cfg

`dy_net_cfg` can call `dy_net_set_config_and_connect` and `dy_net_clear_config_and_disconnect` while reusing the same
event handlers. Ensure `dy_net_cfg_init()` runs **before** initializing Bluetooth (see its README) and after
`dy_net_init()` so that Wi‑Fi primitives are ready.

## Logging

All log messages are tagged `DY_NET`. Adjust verbosity via standard ESP‑IDF log level configuration.

## Future ideas

- Expose a lightweight status getter.
- Configurable reconnect strategy (backoff, max attempts).
- Support for WPA3 / enterprise credential variants.
