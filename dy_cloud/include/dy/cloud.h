#ifndef DY_CLOUD_H
#define DY_CLOUD_H

#define DY_CLOUD_RESP_TIME_TZ_LEN 32
#define DY_CLOUD_RESP_TIME_TZ_DATA_LEN 32

#define DY_CLOUD_RESP_WEATHER_TITLE_DATA_LEN 32

typedef struct {
    char tz[DY_CLOUD_RESP_TIME_TZ_LEN];       // timezone
    char tzd[DY_CLOUD_RESP_TIME_TZ_DATA_LEN]; // timezone data in POSIX format
    unsigned long int ts;                     // timestamp
} dy_cloud_resp_time_t;

typedef struct {
    char title[DY_CLOUD_RESP_WEATHER_TITLE_DATA_LEN];
    int8_t temp;
    int8_t feels;
    uint16_t pressure;
    uint8_t humidity;
} dy_cloud_resp_weather_t;

dy_err_t dy_cloud_time(dy_cloud_resp_time_t *res);

dy_err_t dy_cloud_weather(dy_cloud_resp_weather_t *res);

#endif // DY_CLOUD_H