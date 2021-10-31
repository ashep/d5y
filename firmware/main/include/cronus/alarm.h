#ifndef CRONUS_ALARM_H
#define CRONUS_ALARM_H

#include "cronus/dtime.h"
#include "driver/gpio.h"

#ifndef APP_ALARM_PIN
#define APP_ALARM_PIN GPIO_NUM_15
#endif

/**
 * Initializes alarm hardware.
 *
 * @param app Application
 * @return
 */
esp_err_t app_alarm_init();

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
void app_alarm_start(app_time_t *time);

/**
 * Stops the alarm.
 *
 * @param app
 */
void app_alarm_stop(app_time_t *time);

#endif  // CRONUS_ALARM_H
