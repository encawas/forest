#include "s3_app.h"

#include "sensor_mock.h"
#include "sensor_real_adapter.h"

#include <string.h>

static s3_sensor_source_t g_sensor_source = S3_SENSOR_SOURCE_MOCK;
static echoforest_state_snapshot_t g_cached_state;
static int g_has_cached_state = 0;

void s3_app_init(
    const echoforest_cloud_config_t *cloud_config,
    const echoforest_cloud_transport_t *transport,
    s3_sensor_source_t sensor_source
) {
    memset(&g_cached_state, 0, sizeof(g_cached_state));
    g_cached_state.mode = ECHOFOREST_MODE_IDLE;
    g_cached_state.forest_state = ECHOFOREST_FOREST_IDLE;
    g_cached_state.backend_online = 0;
    g_cached_state.s3_online = 1;
    g_has_cached_state = 0;
    g_sensor_source = sensor_source;

    echoforest_cloud_client_init(cloud_config, transport);
    (void)echoforest_cloud_post_heartbeat();
}

static int read_environment(echoforest_environment_t *out_environment) {
    if (g_sensor_source == S3_SENSOR_SOURCE_REAL) {
        return s3_sensor_real_read(out_environment);
    }
    return s3_sensor_mock_read(out_environment);
}

int s3_app_sample_upload_and_refresh(s3_screen_frame_t *out_frame) {
    echoforest_environment_t environment;
    echoforest_state_snapshot_t next_state;

    if (read_environment(&environment) != 0) {
        if (out_frame != 0) {
            s3_render_main_screen(g_has_cached_state ? &g_cached_state : 0, out_frame);
        }
        return -1;
    }

    if (echoforest_cloud_post_environment(&environment) != 0) {
        g_cached_state.backend_online = 0;
        if (out_frame != 0) {
            s3_render_main_screen(g_has_cached_state ? &g_cached_state : 0, out_frame);
        }
        return -1;
    }

    if (echoforest_cloud_get_state(&next_state) != 0) {
        g_cached_state.backend_online = 0;
        if (out_frame != 0) {
            s3_render_main_screen(g_has_cached_state ? &g_cached_state : 0, out_frame);
        }
        return -1;
    }

    next_state.backend_online = 1;
    next_state.s3_online = 1;
    g_cached_state = next_state;
    g_has_cached_state = 1;

    if (out_frame != 0) {
        s3_render_main_screen(&g_cached_state, out_frame);
    }

    return 0;
}

const echoforest_state_snapshot_t *s3_app_cached_state(void) {
    return g_has_cached_state ? &g_cached_state : 0;
}
