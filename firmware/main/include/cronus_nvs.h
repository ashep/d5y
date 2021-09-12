#ifndef CRONUS_NVS_H
#define CRONUS_NVS_H

#define CRONUS_NVS_K_MAX_BRIGHTNESS "scr_max_bri"

/**
 * @brief Initializes the NVS.
 *
 * @param app Application
 * @return
 */
esp_err_t app_nvs_init(app_t *app);

/**
 * Sets an uint8 value.
 *
 * @param app   Application
 * @param key   Key
 * @param value Value
 * @return
 */
esp_err_t app_nvs_set_u8(app_t *app, const char *key, uint8_t value);

/**
 * Gets an uint8 value.
 *
 * @param app   Application
 * @param key   Key
 * @param value Value
 * @return
 */
esp_err_t app_nvs_get_u8(app_t *app, const char *key, uint8_t *value);

/**
 * @brief Erases all values from the NVS.
 *
 * @param app Application
 * @return
 */
esp_err_t app_nvs_erase(app_t *app);

#endif //CRONUS_NVS_H
