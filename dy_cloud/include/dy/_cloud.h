#ifndef _CRONUS_CLOUD_H
#define _CRONUS_CLOUD_H

#define LTAG "DY_CLOUD"

#define MAX_HTTP_RSP_LEN 1024

#define HTTP_USER_AGENT_FMT "cronus/%d.%d.%d"
#define HTTP_BEARER_TOKEN "tbd"
#define HTTP_REQ_TIMEOUT_MS 5000

#define API_URL_TIME "http://api.d5y.xyz/v2/time"
#define API_URL_WEATHER "http://api.d5y.xyz/v2/weather"

dy_err_t http_request(const char *url, char *rsp);

#endif