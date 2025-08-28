#ifndef DY_NET_H
#define DY_NET_H

#include "dy/error.h"

void dy_net_set_config_and_connect(const char *ssid, const char *password);

void dy_net_clear_config_and_disconnect();

dy_err_t dy_net_init();

#endif  // DY_NET_H
