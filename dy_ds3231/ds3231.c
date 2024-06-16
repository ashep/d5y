#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

#include "dy/error.h"
#include "dy/ds3231.h"

dy_err_t dy_ds3231_read(dy_ds3231_handle_t *hdl, dy_ds3231_data_t *data) {
    esp_err_t esp_err;
    uint8_t buf[DY_DS3231_REG_SZ_READ];

    // At first, we must point to a register address which we want to start read from: from the beginning.
    buf[0] = 0x0;
    esp_err = i2c_master_transmit(hdl->device, buf, 1, -1);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "write register address failed: %s", esp_err_to_name(esp_err));
    }

    esp_err = i2c_master_receive(hdl->device, buf, DY_DS3231_REG_SZ_READ, -1);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "i2c_master_receive failed: %s", esp_err_to_name(esp_err));
    }

    // See page 11 for data format details:
    // https://www.analog.com/media/en/technical-documentation/data-sheets/ds3231.pdf

    // Second
    data->sec = (buf[DY_DS3231_REG_SEC] >> 4) * 10 + (0x0f & buf[DY_DS3231_REG_SEC]);

    // Minute
    data->min = (buf[DY_DS3231_REG_MIN] >> 4) * 10 + (0x0f & buf[DY_DS3231_REG_MIN]);

    // Hour
    data->time_12 = buf[DY_DS3231_REG_HOUR] >> 6;                    // 6th bit is 12-hour mode flag
    if (data->time_12) {
        data->time_pm = (0x20 & buf[DY_DS3231_REG_HOUR]) >> 5;       // 5th bit is AM/PM flag
        data->hour = ((0x10 & buf[DY_DS3231_REG_HOUR]) >> 4) * 10;   // 4th bit is 10-hours counter
        data->hour += 0x0f & buf[DY_DS3231_REG_HOUR];                // bits 0-3 are 1-hour counter
    } else {
        data->hour = ((0x20 & buf[DY_DS3231_REG_HOUR]) >> 5) * 20;   // 5th bit is 20-hours counter
        data->hour += ((0x10 & buf[DY_DS3231_REG_HOUR]) >> 4) * 10;  // 4th bit is 10-hours counter
        data->hour += 0x0f & buf[DY_DS3231_REG_HOUR];                // bits 0-3 are 1-hour counter
    }

    // Day of the week
    data->dow = buf[DY_DS3231_REG_DOW];

    // Day of the month
    data->day = (buf[DY_DS3231_REG_DAY] >> 4) * 10;  // bits 4-5 are 10-days counter
    data->day += 0x0f & buf[DY_DS3231_REG_DAY];      // bits 0-3 are 1-day counter

    // Month
    data->mon = (0x1 & (buf[DY_DS3231_REG_MONTH] >> 4)) * 10;  // 4th bit is 10-month counter
    data->mon += 0x0f & buf[DY_DS3231_REG_MONTH];              // bits 0-3 are 1-month counter

    // Year
    data->year = (buf[DY_DS3231_REG_YEAR] >> 4) * 10;  // bits 7-4 are 10-year counter
    data->year += (0x0f & buf[DY_DS3231_REG_YEAR]);    // bits 0-3 are 1-year counter

    // Alarm 1, second
    data->alarm_1_sec = ((buf[DY_DS3231_REG_ALM_1_SEC] >> 4) & 0x47) * 10 +
                        (buf[DY_DS3231_REG_ALM_1_SEC] & 0x0f);

    // Alarm 1, minute
    data->alarm_1_min = ((buf[DY_DS3231_REG_ALM_1_MIN] >> 4) & 0x47) * 10 +
                        (buf[DY_DS3231_REG_ALM_1_MIN] & 0x0f);

    // Alarm 1, hour
    data->alarm_1_12 = (buf[DY_DS3231_REG_ALM_1_HOUR] >> 6) & 0x1;                // 6th bit is 12-hour mode flag
    if (data->alarm_1_12) {
        data->alarm_1_pm = (buf[DY_DS3231_REG_ALM_1_HOUR] >> 5) & 0x1;            // 5th bit is AM/PM flag
        data->alarm_1_hour = ((buf[DY_DS3231_REG_ALM_1_HOUR] >> 4) & 0x1) * 10;   // 4th bit is 10-hours counter
        data->alarm_1_hour += 0x0f & buf[DY_DS3231_REG_ALM_1_HOUR];               // bits 0-3 are 1-hour counter
    } else {
        data->alarm_1_hour = ((0x20 & buf[DY_DS3231_REG_ALM_1_HOUR]) >> 5) * 20;  // 5th bit is 20-hours counter
        data->alarm_1_hour += ((0x10 & buf[DY_DS3231_REG_ALM_1_HOUR]) >> 4) * 10; // 4th bit is 10-hours counter
        data->alarm_1_hour += 0x0f & buf[DY_DS3231_REG_ALM_1_HOUR];               // bits 0-3 are 1-hour counter
    }

    // TODO: alarm 2
    // TODO: control status
    // TODO: control status
    // TODO: aging offset

    // Temperature
    data->temp = 0x7f & buf[DY_DS3231_REG_TEMP_MSB];         // bits 6-0 of the byte 17 are integer part without sign
    data->temp += (buf[DY_DS3231_REG_TEMP_MSB] >> 6) * 0.25; // bits 7-6 of the byte 18 are fractional 0.25-part
    if (buf[DY_DS3231_REG_TEMP_MSB] >> 7) {                  // 7th bit of 17th byte is the sign
        data->temp *= -1;
    }

    return dy_ok();
}

dy_err_t dy_ds3231_write(dy_ds3231_handle_t *hdl, const dy_ds3231_data_t *data) {
    uint8_t buf[DY_DS3231_REG_SZ_WRITE];

    // See page 11 for data format details:
    // https://www.analog.com/media/en/technical-documentation/data-sheets/ds3231.pdf

    // First byte is the register address where the write operation starts.
    // Since we're going to write all the data sequentially, we're always start from seconds, which is 0x0.
    buf[0] = 0x0;

    // Second
    buf[1] = ((data->sec / 10) << 4) | (data->sec % 10);

    // Minute
    buf[2] = ((data->min / 10) << 4) | (data->min % 10);

    // Hour
    buf[3] = data->time_12 << 6;
    if (data->time_12) {
        buf[3] |= (data->time_pm << 5) | ((data->hour / 10) << 4) | (data->hour % 10);
    } else {
        buf[3] |= ((data->hour / 20) << 5) | ((data->hour / 10) << 4) | (data->hour % 10);
    }

    // Day of the week
    buf[4] = data->dow;

    // Day of the month
    buf[5] = ((data->day / 10) << 4) | (data->day % 10);

    // Month
    buf[6] = ((data->mon / 10) << 4) | (data->mon % 10);

    // Year
    buf[7] = ((data->year / 10) << 4) | (data->year % 10);

    // Alarm 1, second
    buf[8] = ((data->alarm_1_sec / 10) << 4) | (data->alarm_1_sec % 10);

    // Alarm 1, minute
    buf[9] = ((data->alarm_1_min / 10) << 4) | (data->alarm_1_min % 10);

    // Alarm 1, hour
    buf[10] = data->alarm_1_12 << 6;
    if (data->alarm_1_12) {
        buf[10] |= (data->alarm_1_pm << 5) |
                                         ((data->alarm_1_hour / 10) << 4) |
                                         (data->alarm_1_hour % 10);
    } else {
        buf[10] |= ((data->alarm_1_hour / 20) << 5) |
                                         ((data->alarm_1_hour / 10) << 4) |
                                         (data->alarm_1_hour % 10);
    }

    // TODO: alarm 1: day, month
    // TODO: alarm 2: all
    // TODO: control status
    // TODO: control status
    // TODO: aging offset

    esp_err_t esp_err = i2c_master_transmit(hdl->device, buf, DY_DS3231_REG_SZ_WRITE, -1);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "i2c_master_transmit failed: %s", esp_err_to_name(esp_err));
    }

    return dy_ok();
}

dy_err_t dy_ds3231_init(int scl_pin, int sda_pin, dy_ds3231_handle_t *hdl) {
    esp_err_t esp_err;

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = 0,
        .scl_io_num =38,
        .sda_io_num =39,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .flags = {.enable_internal_pullup = 0},
    };

    esp_err = i2c_new_master_bus(&bus_cfg, &hdl->bus);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "i2c_new_master_bus failed: %s", esp_err_to_name(esp_err));
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = DY_DS3231_I2C_ADDR,
        .scl_speed_hz = 100000,
    };

    esp_err = i2c_master_bus_add_device(hdl->bus, &dev_cfg, &hdl->device);
    if (esp_err != ESP_OK) {
        return dy_err(DY_ERR_FAILED, "i2c_master_bus_add_device failed: %s", esp_err_to_name(esp_err));
    }

    return dy_ok();
}