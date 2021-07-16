#ifndef CRONUS_KEYBOARD_H
#define CRONUS_KEYBOARD_H

#ifndef APP_BTN_A
#define APP_BTN_A GPIO_NUM_0
#endif

#ifndef APP_BTN_A_MODE
#define APP_BTN_A_MODE AESPL_BUTTON_PRESS_LOW
#endif

#ifndef APP_BTN_B
#define APP_BTN_B GPIO_NUM_2
#endif

#ifndef APP_BTN_B_MODE
#define APP_BTN_B_MODE AESPL_BUTTON_PRESS_LOW
#endif

/**
 * Initializes keyboard related things.
 *
 * @param app Application
 * @return
 */
esp_err_t app_keyboard_init(app_t *app);

#endif // CRONUS_KEYBOARD_H
