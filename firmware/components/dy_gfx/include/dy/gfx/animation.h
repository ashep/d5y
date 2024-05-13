/**
 * @author    Oleksandr Shepetko <a@shepetko.com>
 * @copyright MIT License
 */

#ifndef DY_GFX_ANIMATION_H
#define DY_GFX_ANIMATION_H

#include <stdio.h>

#include "dy/gfx/buffer.h"

/**
 * Animation state commands.
 */
typedef enum {
    DY_GFX_ANIM_CONTINUE,
    DY_GFX_ANIM_RESTART,
    DY_GFX_ANIM_STOP,
} dy_gfx_anim_state_t;

/**
 * Animation callback.
 */
typedef dy_gfx_anim_state_t (*dy_gfx_animator_t)(void *args, uint32_t frame_n);

/**
 * Animation state.
 */
typedef struct {
    dy_gfx_animator_t animator;
    void *args;
    uint8_t fps;
    uint32_t frame_n;
    dy_gfx_anim_state_t state;

} dy_gfx_animation_t;

/**
 * Starts animation.
 */
dy_gfx_animation_t *dy_gfx_animate(dy_gfx_animator_t fn, void *args, uint8_t fps);

#endif // DY_GFX_ANIMATION_H
