# dy_net_cfg

`dy_net_cfg` provides a Bluetooth interface for network setup management.

## API

### dy_err_t dy_net_cfg_init()

This module must be initialized **before** `dy_bt`.

```c
dy_err_t err;
if (dy_is_err(err = dy_net_cfg_init())) {
    ESP_LOGE(LTAG, "dy_net_cfg_init: %s", dy_err_str(err));
}
```

### bool dy_net_cfg_net_connected()

Returns true if the device is connected to a network, false otherwise.

## Bluetooth API

### Read 0xff01

Returns connection state, last operation error code, connected SSID, and scanned SSIDs. 

- Byte 0: state and error reason:
  - Bits 0-3: connection state:
    - `DY_NET_CFG_CONN_DISCONNECTED`;
    - `DY_NET_CFG_CONN_CONNECTING`;
    - `DY_NET_CFG_CONN_CONNECTED`;
    - `DY_NET_CFG_CONN_ERROR`.
  - Bits 4-7: connection error reason (reserved for future use):
    - `DY_NET_CFG_ERR_NONE`.
- Bytes 1-32: connected SSID, if any.
- Bytes 33-64: scanned SSID 1, if any.
- Bytes 65-96: scanned SSID 2, if any.
- Bytes 97-128: scanned SSID 3, if any.
- Bytes 129-160: scanned SSID 4, if any.
- Bytes 161-192: scanned SSID 5, if any.

### Write 0xff01

- Byte 0: operation code:
  - `DY_NET_CFG_OP_NOP`: do nothing;
  - `DY_NET_CFG_OP_SCAN`: start SSIDs scan;
  - `DY_NET_CFG_OP_CONNECT`: connect to an access point;
  - `DY_NET_CFG_OP_CLEAR_CONFIG`: disconnect and forget access point.
- Bytes 1-32: (optional) SSID to connect to, if the operation is `DY_NET_CFG_OP_CONNECT`.
- Bytes 33-97: SSID password, if the operation is `DY_NET_CFG_OP_CONNECT`.
