#ifndef DY_CLOUD_H
#define DY_CLOUD_H

#define DY_CLOUD_RESP_TIME_TZ_LEN 32
#define DY_CLOUD_RESP_TIME_TZ_DATA_LEN 32

#define DY_CLOUD_RESP_GEO_COUNTRY_LEN 32
#define DY_CLOUD_RESP_GEO_REGION_LEN 32
#define DY_CLOUD_RESP_GEO_CITY_LEN 32
#define DY_CLOUD_RESP_GEO_TIMEZONE_LEN 32

typedef struct {
    char tz[DY_CLOUD_RESP_TIME_TZ_LEN];
    char tz_data[DY_CLOUD_RESP_TIME_TZ_DATA_LEN];
    int ts;
} dy_cloud_resp_time_t;

typedef struct {
    char country[DY_CLOUD_RESP_GEO_COUNTRY_LEN];
    char region[DY_CLOUD_RESP_GEO_REGION_LEN];
    char city[DY_CLOUD_RESP_GEO_CITY_LEN];
    char timezone[DY_CLOUD_RESP_GEO_TIMEZONE_LEN];
} dy_cloud_resp_geo_t;

typedef struct {
    char title[64];
    float temp;
    float feels;
    int pressure;
    int humidity;
} dy_cloud_resp_weather_t;

typedef struct {
    dy_cloud_resp_time_t time;
    dy_cloud_resp_geo_t geo;
    dy_cloud_resp_weather_t weather;
} dy_cloud_resp_me_t;

dy_err_t dy_cloud_init();

dy_err_t dy_cloud_get_me(dy_cloud_resp_me_t *res);

#endif // DY_CLOUD_H