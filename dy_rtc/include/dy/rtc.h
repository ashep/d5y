#ifndef DY_RTC_H
#define DY_RTC_H

#include "dy/error.h"
#include "dy/ds3231.h"

dy_err_t dy_rtc_init(dy_ds3231_handle_t *ds3231_hdl);

#endif // DY_RTC_H