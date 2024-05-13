#include "dy/error.h"
#include "dy/gfx/buffer.h"

#include "dy/display.h"

static dy_display_driver_t drv[DY_DISPLAY_ID_MAX];

/**
 * For internal use only.
 */
dy_err_t set_driver(uint8_t id, void *cfg, buf_drawer_t draw) {
    if (id >= DY_DISPLAY_ID_MAX) {
        return dy_error(DY_ERR_INVALID_ARG, "display id must be lower than %d", DY_DISPLAY_ID_MAX);
    }

    if (cfg == NULL) {
        return dy_error(DY_ERR_INVALID_ARG, "cfg is null");
    }

    if (draw == NULL) {
        return dy_error(DY_ERR_INVALID_ARG, "draw is null");
    }

    drv[id].cfg = cfg;
    drv[id].draw = draw;

    return dy_ok();
}

dy_err_code_t dy_display_out(uint8_t id, dy_gfx_buf_t *buf) {
    if (id >= DY_DISPLAY_ID_MAX) {
        return DY_ERR_INVALID_ARG;
    }

    if (drv[id].draw == NULL || drv[id].cfg == NULL || buf == NULL) {
        return DY_ERR_INVALID_ARG;
    }

    return drv[id].draw(drv[id].cfg, buf);
}
