#ifndef CRONUS_TIME_H
#define CRONUS_TIME_H

#include "semphr.h"
#include "nvs.h"

#include "aespl_ds3231.h"

#include "cronus/mode.h"

#ifndef APP_DS3231_SDA_PIN
#define APP_DS3231_SDA_PIN GPIO_NUM_4
#endif

#ifndef APP_DS3231_SDA_PULLUP
#define APP_DS3231_SDA_PULLUP GPIO_PULLUP_DISABLE
#endif

#ifndef APP_DS3231_SCL_PIN
#define APP_DS3231_SCL_PIN GPIO_NUM_5
#endif

#ifndef APP_DS3231_SCL_PULLUP
#define APP_DS3231_SCL_PULLUP GPIO_PULLUP_DISABLE
#endif

#ifndef APP_DS3231_TIMEOUT
#define APP_DS3231_TIMEOUT 100  // milliseconds
#endif

#define APP_SECOND 1000
#define APP_MINUTE 60000

typedef struct {
    SemaphoreHandle_t mux;
    app_mode_t *app_mode;
    aespl_ds3231_t *rtc;
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t dow;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t alarm_hour;
    uint8_t alarm_minute;
    uint8_t alarm_enabled;
    uint8_t alarm_started;
    uint8_t flush_to_rtc; // whether is datetime and alarm settings should be written toe RTC
    xTaskHandle alarm_task;
} app_time_t;

app_time_t *app_time_init(app_mode_t *app_mode, nvs_handle_t nvs);

#endif //CRONUS_TIME_H
