#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dy/gfx/animation.h"

static void animate_task(void *args) {
    dy_gfx_animation_t *anim = (dy_gfx_animation_t *) args;
    TickType_t delay = pdMS_TO_TICKS(1000 / anim->fps);

    for (;;) {
        if (anim->state == DY_GFX_ANIM_RESTART) {
            anim->frame_n = 0;
            anim->state = DY_GFX_ANIM_CONTINUE;
        }

        if (anim->state == DY_GFX_ANIM_CONTINUE) {
            anim->state = anim->animator(anim->args, anim->frame_n++);
            vTaskDelay(delay);
        }

        if (anim->state == DY_GFX_ANIM_STOP) {
            free(anim);
            break;
        }
    }

    vTaskDelete(NULL);
}

dy_gfx_animation_t *dy_gfx_animate(dy_gfx_animator_t fn, void *args, uint8_t fps) {
    dy_gfx_animation_t *anim = malloc(sizeof(dy_gfx_animation_t));
    if (!anim) {
        return NULL;
    }

    anim->animator = fn;
    anim->args = args;
    anim->fps = fps;
    anim->frame_n = 0;
    anim->state = DY_GFX_ANIM_CONTINUE;

    xTaskCreate(animate_task, "animation", 4096, (void *) anim, 0, NULL);

    return anim;
}
