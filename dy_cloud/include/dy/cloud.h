#pragma once

#define DY_CLOUD_WEATHER_TITLE_LEN 32
#define DY_CLOUD_TIME_TZ_LEN 32
#define DY_CLOUD_TIME_TZ_DATA_LEN 32

#define DY_CLOUD_EV_BASE "DY_CLOUD"

typedef enum {
    DY_CLOUD_EV_TIME_UPDATED,
    DY_CLOUD_EV_WEATHER_UPDATED,
} dy_cloud_event_id_t;

typedef struct {
    char tz[DY_CLOUD_TIME_TZ_LEN];       // timezone
    char tzd[DY_CLOUD_TIME_TZ_DATA_LEN]; // timezone data in POSIX format
    unsigned long int ts;                     // timestamp
} dy_cloud_time_t;

typedef struct {
    char title[DY_CLOUD_WEATHER_TITLE_LEN + 1];
    int8_t temp;
    int8_t feels;
    uint16_t pressure;
    uint8_t humidity;
} dy_cloud_weather_t;

dy_err_t dy_cloud_time_start_scheduler();

dy_err_t dy_cloud_weather_start_scheduler();

dy_err_t dy_cloud_update_start_scheduler(bool allow_alpha);
