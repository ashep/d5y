#include "dy/error.h"
#include "dy/widget.h"
#include "dy/widget/clock.h"

dy_err_t dy_widget_init() {
    dy_err_t err;

    if (dy_nok(err = dy_widget_clock_init())) {
        return dy_err_pfx("dy_widget_clock_init failed", err);
    }

    return dy_ok();
}
