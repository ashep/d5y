#include "dy/error.h"
#include "dy/gfx/gfx.h"
#include "dy/display.h"

static dy_display_driver_t drivers[DY_DISPLAY_ID_MAX];
static uint8_t brightness[DY_DISPLAY_ID_MAX] = {0};

dy_err_t dy_display_set_driver(uint8_t id, dy_display_driver_t drv) {
    if (id >= DY_DISPLAY_ID_MAX) {
        return dy_err(DY_ERR_INVALID_ARG, "display id must be lower than %d", DY_DISPLAY_ID_MAX);
    }

    drivers[id].cfg = drv.cfg;
    drivers[id].write = drv.write;
    drivers[id].set_brightness = drv.set_brightness;
    drivers[id].refresh = drv.refresh;

    return dy_ok();
}

dy_err_t dy_display_write(uint8_t id, dy_gfx_buf_t *buf) {
    if (id >= DY_DISPLAY_ID_MAX) {
        return dy_err(DY_ERR_INVALID_ARG, "display id must not be greater than %d", DY_DISPLAY_ID_MAX);
    }

    if (buf == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "null buffer");
    }

    if (drivers[id].write == NULL || drivers[id].cfg == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "display driver is not initialized");
    }

    return drivers[id].write(drivers[id].cfg, buf);
}

dy_err_t dy_display_set_brightness(uint8_t id, uint8_t value) {
    if (id >= DY_DISPLAY_ID_MAX) {
        return dy_err(DY_ERR_INVALID_ARG, "id must not be greater than %d", DY_DISPLAY_ID_MAX);
    }

    if (value > DY_DISPLAY_BRI_MAX) {
        return dy_err(DY_ERR_INVALID_ARG, "brightness must not be greater than %d", DY_DISPLAY_BRI_MAX);
    }

    if (drivers[id].set_brightness == NULL || drivers[id].cfg == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "driver is not initialized");
    }

    dy_err_t err = drivers[id].set_brightness(drivers[id].cfg, value);
    if (!dy_is_err(err)) {
        brightness[id] = value;
    }

    return err;
}

uint8_t dy_display_get_brightness(uint8_t id) {
    if (id >= DY_DISPLAY_ID_MAX) {
        return 0; // Invalid ID, return 0 as a safe default
    }

    return brightness[id];
}

dy_err_t dy_display_refresh(uint8_t id) {
    if (id >= DY_DISPLAY_ID_MAX) {
        return dy_err(DY_ERR_INVALID_ARG, "display id must not be greater than %d", DY_DISPLAY_ID_MAX);
    }

    if (drivers[id].refresh == NULL || drivers[id].cfg == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "display driver is not initialized");
    }

    return drivers[id].refresh(drivers[id].cfg);
}
