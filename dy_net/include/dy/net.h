#ifndef DY_NET_H
#define DY_NET_H

#include "dy/error.h"

void dy_net_set_config_and_connect(const char *ssid, const char *password);

void dy_net_clear_config_and_disconnect();

/**
 * @brief Starts an asynchronous Wi-Fi scan; results arrive with WIFI_EVENT_SCAN_DONE.
 *
 * If the station is not connected, any ongoing connection attempt is aborted and reconnects are paused until the scan
 * is done, since ESP-IDF does not allow scanning while the station is connecting.
 */
dy_err_t dy_net_scan_start();

dy_err_t dy_net_init();

#endif  // DY_NET_H
