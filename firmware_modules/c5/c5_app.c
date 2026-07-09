#include "c5_app.h"

#include <string.h>

static echoforest_state_snapshot_t g_cached_state;
static int g_has_cached_state = 0;

void c5_app_init(
    const echoforest_cloud_config_t *cloud_config,
    const echoforest_cloud_transport_t *transport
) {
    memset(&g_cached_state, 0, sizeof(g_cached_state));
    g_cached_state.mode = ECHOFOREST_MODE_IDLE;
    g_cached_state.forest_state = ECHOFOREST_FOREST_IDLE;
    g_cached_state.backend_online = 0;
    g_cached_state.s3_online = 0;
    g_has_cached_state = 0;

    echoforest_cloud_client_init(cloud_config, transport);
    (void)echoforest_cloud_post_heartbeat();
}

int c5_app_handle_touch(c5_touch_zone_t zone, const char *goal) {
    c5_touch_action_result_t action_result = c5_touch_zone_to_action(zone);
    if (!action_result.has_action) {
        return 0;
    }

    return echoforest_cloud_post_control(action_result.action, goal);
}

int c5_app_poll_backend(c5_screen_frame_t *out_frame, c5_speaker_prompt_t *out_prompt) {
    echoforest_state_snapshot_t next_state;
    echoforest_forest_state_t previous_forest_state = g_has_cached_state
        ? g_cached_state.forest_state
        : ECHOFOREST_FOREST_IDLE;

    if (out_prompt != 0) {
        out_prompt->should_prompt = 0;
        out_prompt->prompt_text = "";
    }

    if (echoforest_cloud_get_state(&next_state) != 0) {
        g_cached_state.backend_online = 0;
        if (out_frame != 0) {
            c5_render_control_screen(g_has_cached_state ? &g_cached_state : 0, out_frame);
        }
        return -1;
    }

    next_state.backend_online = 1;
    g_cached_state = next_state;
    g_has_cached_state = 1;

    if (out_frame != 0) {
        c5_render_control_screen(&g_cached_state, out_frame);
    }

    if (out_prompt != 0) {
        *out_prompt = c5_speaker_prompt_for_transition(
            previous_forest_state,
            g_cached_state.forest_state
        );
    }

    return 0;
}

const echoforest_state_snapshot_t *c5_app_cached_state(void) {
    return g_has_cached_state ? &g_cached_state : 0;
}
