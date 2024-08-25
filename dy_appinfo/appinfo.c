#include <stdio.h>
#include <string.h>

#include "dy/appinfo.h"

static char owner[DY_CLOUD_APPINFO_APP_OWNER_LEN + 1];
static char name[DY_CLOUD_APPINFO_APP_NAME_LEN + 1];
static char version[DY_CLOUD_APPINFO_APP_VER_LEN + 1];
static char arch[DY_CLOUD_APPINFO_APP_ARCH_LEN + 1];
static char hw_id[DY_CLOUD_APPINFO_APP_HW_ID_LEN + 1];
static char auth[DY_CLOUD_APPINFO_APP_AUTH_LEN + 1];

static char id[DY_CLOUD_APPINFO_APP_ID_LEN + 1];
static char user_agent[DY_CLOUD_APPINFO_USER_AGENT_LEN + 1];

void dy_appinfo_set(dy_appinfo_info_t *info) {
    strncpy(owner, info->owner, DY_CLOUD_APPINFO_APP_OWNER_LEN);
    strncpy(name, info->name, DY_CLOUD_APPINFO_APP_NAME_LEN);
    strncpy(version, info->version, DY_CLOUD_APPINFO_APP_VER_LEN);
    strncpy(arch, info->arch, DY_CLOUD_APPINFO_APP_ARCH_LEN);
    strncpy(hw_id, info->hw_id, DY_CLOUD_APPINFO_APP_HW_ID_LEN);
    strncpy(auth, info->auth, DY_CLOUD_APPINFO_APP_AUTH_LEN);

    snprintf(id, DY_CLOUD_APPINFO_APP_ID_LEN, "%s/%s", owner, name);
    snprintf(user_agent, DY_CLOUD_APPINFO_USER_AGENT_LEN, "%s/%s", id, version);
}

dy_appinfo_info_t dy_appinfo_get() {
    return (dy_appinfo_info_t) {
        .owner = owner,
        .name = name,
        .version = version,
        .arch = arch,
        .hw_id = hw_id,
        .auth = auth,
    };
}

dy_appinfo_ext_info_t dy_appinfo_get_ext() {
    return (dy_appinfo_ext_info_t) {
        .id = id,
        .user_agent = user_agent,
    };
}