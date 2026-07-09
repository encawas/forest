#pragma once

#include "../common/cloud_client.h"
#include "control_screen_renderer.h"
#include "speaker_events.h"
#include "touch_actions.h"

#ifdef __cplusplus
extern "C" {
#endif

void c5_app_init(
    const echoforest_cloud_config_t *cloud_config,
    const echoforest_cloud_transport_t *transport
);

int c5_app_handle_touch(c5_touch_zone_t zone, const char *goal);
int c5_app_handle_touch_point(int x, int y, const c5_touch_layout_t *layout, const char *goal);
int c5_app_poll_backend(c5_screen_frame_t *out_frame, c5_speaker_prompt_t *out_prompt);
const echoforest_state_snapshot_t *c5_app_cached_state(void);

#ifdef __cplusplus
}
#endif
