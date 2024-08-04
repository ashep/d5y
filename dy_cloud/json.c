#include "cJSON.h"
#include "dy/error.h"

dy_err_t json_err() {
    dy_err_t err;

    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
        err = dy_err(DY_ERR_JSON_PARSE, "%s", error_ptr);
    } else {
        err = dy_err(DY_ERR_JSON_PARSE, "reason unknown");
    }

    return err;
}
