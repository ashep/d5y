#pragma once

#define DY_CLOUD_APPINFO_APP_OWNER_LEN 16
#define DY_CLOUD_APPINFO_APP_NAME_LEN 16
#define DY_CLOUD_APPINFO_APP_VER_LEN 16
#define DY_CLOUD_APPINFO_APP_ARCH_LEN 16
#define DY_CLOUD_APPINFO_APP_HW_ID_LEN 16
#define DY_CLOUD_APPINFO_APP_AUTH_LEN 32

#define DY_CLOUD_APPINFO_APP_ID_LEN 34
#define DY_CLOUD_APPINFO_USER_AGENT_LEN 52

typedef struct {
    const char *owner;
    const char *name;
    const char *version;
    const char *arch;
    const char *hw_id;
    const char *auth;
} dy_appinfo_info_t;


typedef struct {
    const char *id;
    const char *user_agent;
} dy_appinfo_ext_info_t;

void dy_appinfo_set(dy_appinfo_info_t *info);

dy_appinfo_info_t dy_appinfo_get();

dy_appinfo_ext_info_t dy_appinfo_get_ext();