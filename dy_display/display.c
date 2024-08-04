#include "dy/error.h"
#include "dy/gfx/gfx.h"

#include "dy/display.h"

static dy_display_driver_t drv[DY_DISPLAY_ID_MAX];

/**
 * For internal use only.
 */
dy_err_t set_driver(
    uint8_t id,
    void *cfg,
    display_drv_write_t write,
    display_drv_set_brightness_t set_bri,
    display_drv_refresh_t refresh
) {
    if (id >= DY_DISPLAY_ID_MAX) {
        return dy_err(DY_ERR_INVALID_ARG, "display id must be lower than %d", DY_DISPLAY_ID_MAX);
    }

    if (cfg == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "cfg is null");
    }

    if (write == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "draw is null");
    }

    drv[id].cfg = cfg;
    drv[id].write = write;
    drv[id].set_brightness = set_bri;
    drv[id].refresh = refresh;

    return dy_ok();
}

dy_err_t dy_display_write(uint8_t id, dy_gfx_buf_t *buf) {
    if (id >= DY_DISPLAY_ID_MAX) {
        return dy_err(DY_ERR_INVALID_ARG, "display id must not be greater than %d", DY_DISPLAY_ID_MAX);
    }

    if (buf == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "null buffer");
    }

    if (drv[id].write == NULL || drv[id].cfg == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "display driver is not initialized");
    }

    return drv[id].write(drv[id].cfg, buf);
}


dy_err_t dy_display_set_brightness(uint8_t id, uint8_t value) {
    if (id >= DY_DISPLAY_ID_MAX) {
        return dy_err(DY_ERR_INVALID_ARG, "display id must not be greater than %d", DY_DISPLAY_ID_MAX);
    }

    if (drv[id].set_brightness == NULL || drv[id].cfg == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "display driver is not initialized");
    }

    return drv[id].set_brightness(drv[id].cfg, value);
}

dy_err_t dy_display_refresh(uint8_t id) {
    if (id >= DY_DISPLAY_ID_MAX) {
        return dy_err(DY_ERR_INVALID_ARG, "display id must not be greater than %d", DY_DISPLAY_ID_MAX);
    }

    if (drv[id].refresh == NULL || drv[id].cfg == NULL) {
        return dy_err(DY_ERR_INVALID_ARG, "display driver is not initialized");
    }

    return drv[id].refresh(drv[id].cfg);
}
