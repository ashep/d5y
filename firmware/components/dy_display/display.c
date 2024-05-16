#include "dy/error.h"
#include "dy/gfx/gfx.h"

#include "dy/display.h"

static dy_display_driver_t drv[DY_DISPLAY_ID_MAX];

/**
 * For internal use only.
 */
dy_err_t set_driver(uint8_t id, void *cfg, display_writer_t writer) {
    if (id >= DY_DISPLAY_ID_MAX) {
        return dy_error(DY_ERR_INVALID_ARG, "display id must be lower than %d", DY_DISPLAY_ID_MAX);
    }

    if (cfg == NULL) {
        return dy_error(DY_ERR_INVALID_ARG, "cfg is null");
    }

    if (writer == NULL) {
        return dy_error(DY_ERR_INVALID_ARG, "draw is null");
    }

    drv[id].cfg = cfg;
    drv[id].write = writer;

    return dy_ok();
}

dy_err_t dy_display_write(uint8_t id, dy_gfx_buf_t *buf) {
    if (id >= DY_DISPLAY_ID_MAX) {
        return dy_error(DY_ERR_INVALID_ARG, "display id must not be greater than %d", DY_DISPLAY_ID_MAX);
    }

    if (buf == NULL) {
        return dy_error(DY_ERR_INVALID_ARG, "null buffer");
    }

    if (drv[id].write == NULL || drv[id].cfg == NULL) {
        return dy_error(DY_ERR_INVALID_ARG, "display driver is not initialized");
    }

    return drv[id].write(drv[id].cfg, buf);
}
