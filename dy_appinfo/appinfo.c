#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "dy/error.h"

#include "dy/appinfo.h"

static bool init = 0;

static char owner[DY_APPINFO_APP_OWNER_LEN + 1];
static char name[DY_APPINFO_APP_NAME_LEN + 1];
static char auth[DY_APPINFO_APP_AUTH_LEN + 1];

static uint8_t v_major;
static uint8_t v_minor;
static uint8_t v_patch;
static uint8_t v_alpha;

static char arch[DY_APPINFO_APP_ARCH_LEN + 1];
static char id[DY_APPINFO_APP_ID_LEN + 1];

dy_err_t dy_appinfo_set(dy_appinfo_info_t *inf) {
    strncpy(owner, inf->owner, DY_APPINFO_APP_OWNER_LEN);
    strncpy(name, inf->name, DY_APPINFO_APP_NAME_LEN);
    strncpy(auth, inf->auth, DY_APPINFO_APP_AUTH_LEN);

    v_major = inf->ver.major;
    v_minor = inf->ver.minor;
    v_patch = inf->ver.patch;
    v_alpha = inf->ver.alpha;

    strncpy(arch, CONFIG_IDF_TARGET, DY_APPINFO_APP_ARCH_LEN);
    snprintf(id, DY_APPINFO_APP_ID_LEN, "%s:%s:%s:%d.%d.%d", owner, name, arch, v_major, v_minor, v_patch);

    if (v_alpha > 0) {
        strcat(id, "-alpha");
        char al_s[4] = {0}; // max 255, plus \0
        utoa(v_alpha, al_s, 10);
        strncat(id, al_s, 4);
    }

    init = true;

    return dy_ok();
}

dy_err_t dy_appinfo_get(dy_appinfo_info_t *dst) {
    if (!init) {
        return dy_err(DY_ERR_NOT_CONFIGURED, "dy_appinfo_set must be called before");
    }

    dst->ver = (dy_appinfo_ver_t) {
        .major = v_major,
        .minor = v_minor,
        .patch = v_patch,
        .alpha = v_alpha,
    };

    dst->owner = owner;
    dst->name = name;
    dst->auth = auth;

    dst->arch = arch;
    dst->id = id;

    return dy_ok();
}
