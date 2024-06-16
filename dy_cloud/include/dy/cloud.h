#ifndef DY_CLOUD_H
#define DY_CLOUD_H

#define DY_CLOUD_RESP_TIME_TZ_LEN 32
#define DY_CLOUD_RESP_TIME_TZ_DATA_LEN 32

typedef struct {
    char tz[DY_CLOUD_RESP_TIME_TZ_LEN];       // timezone
    char tzd[DY_CLOUD_RESP_TIME_TZ_DATA_LEN]; // timezone data in POSIX format
    unsigned long int ts;                     // timestamp
} dy_cloud_resp_time_t;

typedef struct {
    char title[64];
    float temp;
    float feels;
    int pressure;
    int humidity;
} dy_cloud_resp_weather_t;

dy_err_t dy_cloud_get_time(dy_cloud_resp_time_t *res);

#endif // DY_CLOUD_H