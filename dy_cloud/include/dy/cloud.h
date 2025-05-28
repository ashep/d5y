#pragma once

#include <time.h>

#define DY_CLOUD_WEATHER_TITLE_LEN 32
#define DY_CLOUD_TIME_TZ_LEN 32
#define DY_CLOUD_TIME_TZ_DATA_LEN 32

#define DY_CLOUD_EV_BASE "DY_CLOUD"

#define DY_CLOUD_WEATHER_ID_UNKNOWN 0
#define DY_CLOUD_WEATHER_ID_CLEAR 1
#define DY_CLOUD_WEATHER_ID_PARTLY_CLOUDY 2
#define DY_CLOUD_WEATHER_ID_CLOUDY 3
#define DY_CLOUD_WEATHER_ID_OVERCAST 4
#define DY_CLOUD_WEATHER_ID_MIST 5
#define DY_CLOUD_WEATHER_ID_LIGHT_RAIN 6
#define DY_CLOUD_WEATHER_ID_MEDIUM_RAIN 7
#define DY_CLOUD_WEATHER_ID_HEAVY_RAIN 8
#define DY_CLOUD_WEATHER_ID_LIGHT_SNOW 9
#define DY_CLOUD_WEATHER_ID_MEDIUM_SNOW 10
#define DY_CLOUD_WEATHER_ID_HEAVY_SNOW 11
#define DY_CLOUD_WEATHER_ID_LIGHT_SLEET 12
#define DY_CLOUD_WEATHER_ID_HEAVY_SLEET 13
#define DY_CLOUD_WEATHER_ID_THUNDERSTORM 14
#define DY_CLOUD_WEATHER_ID_FOG 15
#define DY_CLOUD_WEATHER_ID_LIGHT_HAIL 16
#define DY_CLOUD_WEATHER_ID_HEAVY_HAIL 17

typedef enum {
    DY_CLOUD_EV_TIME_UPDATED,
    DY_CLOUD_EV_WEATHER_UPDATED,
} dy_cloud_event_id_t;

typedef struct {
    char tz[DY_CLOUD_TIME_TZ_LEN];       // timezone
    char tzd[DY_CLOUD_TIME_TZ_DATA_LEN]; // timezone data in POSIX format
    unsigned long int ts;                // timestamp
} dy_cloud_time_t;

typedef struct {
    uint8_t id;
    char title[DY_CLOUD_WEATHER_TITLE_LEN + 1];
    bool is_day;
    int8_t temp;
    int8_t feels;
    time_t ts;
} dy_cloud_weather_t;

dy_err_t dy_cloud_time_start_scheduler();

dy_err_t dy_cloud_weather_start_scheduler();

dy_err_t dy_cloud_update_start_scheduler(bool allow_alpha);
