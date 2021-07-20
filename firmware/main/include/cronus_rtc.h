#ifndef CRONUS_RTC_H
#define CRONUS_RTC_H

#include "cronus_main.h"

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
#define APP_MINUTE APP_SECOND * 60
#define APP_HOUR APP_MINUTE * 60


/**
 * Initializes RTC related things.
 *
 * @param app Application
 * @return
 */
esp_err_t app_rtc_init(app_t *app);

/**
 * Sets app->time from RTC values.
 *
 * @param app Application
 * @return
 */
esp_err_t set_app_time_from_rtc(app_t *app);

/**
 * Sets RTC values from app->time.
 *
 * @param app Application
 * @return
 */
esp_err_t set_rtc_from_app_time(app_t *app);

#endif // CRONUS_RTC_H
