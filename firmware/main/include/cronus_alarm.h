#ifndef CRONUS_CRONUS_ALARM_H
#define CRONUS_CRONUS_ALARM_H

#ifndef APP_ALARM_PIN
#define APP_ALARM_PIN GPIO_Pin_15
#endif

/**
 * Initializes alarm hardware.
 *
 * @param app Application
 * @return
 */
esp_err_t app_alarm_init(app_t *app);

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
