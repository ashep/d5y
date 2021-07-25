#ifndef CRONUS_CRONUS_ALARM_H
#define CRONUS_CRONUS_ALARM_H

#include "gpio.h"

#ifndef APP_ALARM_PIN
#define APP_ALARM_PIN GPIO_NUM_15
#endif

/**
 * Initializes alarm hardware.
 *
 * @param app Application
 * @return
 */
esp_err_t app_alarm_init(app_t *app);

/**
 * Issues single alarm beep.
 *
 * @param app
 */
void app_alarm_beep();

/**
 * Starts the alarm.
 *
 * @param app
 */
void app_alarm_start(app_t *app);

/**
 * Stops the alarm.
 *
 * @param app
 */
void app_alarm_stop(app_t *app);

#endif // CRONUS_CRONUS_ALARM_H
