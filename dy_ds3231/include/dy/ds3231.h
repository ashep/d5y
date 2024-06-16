#ifndef DY_DS3231_H
#define DY_DS3231_H

#include "driver/gpio.h"
#include "driver/i2c_master.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "dy/error.h"

#define DY_DS3231_I2C_ADDR 0x68
#define DY_DS3231_REG_SZ_READ 19
#define DY_DS3231_REG_SZ_WRITE 11

/**
 * DS3231 registers addresses
 */
typedef enum {
    DY_DS3231_REG_SEC,
    DY_DS3231_REG_MIN,
    DY_DS3231_REG_HOUR,
    DY_DS3231_REG_DOW,
    DY_DS3231_REG_DAY,
    DY_DS3231_REG_MONTH,
    DY_DS3231_REG_YEAR,
    DY_DS3231_REG_ALM_1_SEC,
    DY_DS3231_REG_ALM_1_MIN,
    DY_DS3231_REG_ALM_1_HOUR,
    DY_DS3231_REG_ALARM_1_DATE,
    DY_DS3231_REG_ALARM_2_MINUTES,
    DY_DS3231_REG_ALARM_2_HOURS,
    DY_DS3231_REG_ALARM_2_DATE,
    DY_DS3231_REG_CONTROL,
    DY_DS3231_REG_CONTROL_STATUS,
    DY_DS3231_REG_AGING_OFFSET,
    DY_DS3231_REG_TEMP_MSB,
    DY_DS3231_REG_TEMP_LSB,
} dy_ds3231_reg_t;

typedef struct {
    i2c_master_bus_handle_t bus;
    i2c_master_dev_handle_t device;
} dy_ds3231_handle_t;

typedef struct {
    bool time_12;  // 12-hour format
    bool time_pm;  // false == AM, true == PM
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t dow;
    uint8_t day;
    uint8_t mon;
    uint8_t year;
    bool alarm_1_12;
    bool alarm_1_pm;
    uint8_t alarm_1_sec;
    uint8_t alarm_1_min;
    uint8_t alarm_1_hour;
    double temp;
} dy_ds3231_data_t;

dy_err_t dy_ds3231_init(int scl_pin, int sda_pin, dy_ds3231_handle_t *hdl);

dy_err_t dy_ds3231_read(dy_ds3231_handle_t *hdl, dy_ds3231_data_t *data);

dy_err_t dy_ds3231_write(dy_ds3231_handle_t *hdl, const dy_ds3231_data_t *data);

#endif
