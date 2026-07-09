#include "sensor_real_adapter.h"

#include <stddef.h>
#include <string.h>

static s3_sensor_driver_t g_driver;
static int g_has_driver = 0;

void s3_sensor_real_adapter_init(const s3_sensor_driver_t *driver) {
    if (driver == NULL) {
        memset(&g_driver, 0, sizeof(g_driver));
        g_has_driver = 0;
        return;
    }

    g_driver = *driver;
    g_has_driver = 1;
}

int s3_sensor_real_read(echoforest_environment_t *out_environment) {
    if (out_environment == NULL || !g_has_driver) {
        return -1;
    }

    if (g_driver.read_light == NULL || g_driver.read_noise == NULL
        || g_driver.read_temp == NULL || g_driver.read_humi == NULL) {
        return -1;
    }

    if (g_driver.read_light(&out_environment->light) != 0) return -1;
    if (g_driver.read_noise(&out_environment->noise) != 0) return -1;
    if (g_driver.read_temp(&out_environment->temp) != 0) return -1;
    if (g_driver.read_humi(&out_environment->humi) != 0) return -1;

    out_environment->updated_at[0] = '\0';
    return 0;
}
