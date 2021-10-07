/**
 * @brief     Cronus Digital Clock
 *
 * @author    Alexander Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#ifndef CRONUS_MAIN_H
#define CRONUS_MAIN_H

#ifndef APP_NAME
#define APP_NAME "Cronus"
#endif

#ifndef APP_VERSION
#define APP_VERSION "1.1"
#endif

#define APP_HW_VER_1 0x1

#ifndef APP_HW_VERSION
#define APP_HW_VERSION APP_HW_VER_1
#endif

/**
 * RTOS app entry point.
 */
void app_main();

#endif // CRONUS_MAIN_H
