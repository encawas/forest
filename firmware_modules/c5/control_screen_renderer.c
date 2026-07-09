#include "control_screen_renderer.h"

#include <stdio.h>
#include <string.h>

static void clear_frame(c5_screen_frame_t *frame) {
    for (int i = 0; i < C5_SCREEN_LINE_COUNT; ++i) {
        frame->lines[i][0] = '\0';
    }
}

void c5_render_control_screen(
    const echoforest_state_snapshot_t *state,
    c5_screen_frame_t *out_frame
) {
    if (out_frame == NULL) {
        return;
    }

    clear_frame(out_frame);

    if (state == NULL) {
        snprintf(out_frame->lines[0], C5_SCREEN_LINE_LEN, "ECHOFOREST C5");
        snprintf(out_frame->lines[1], C5_SCREEN_LINE_LEN, "WAIT BACKEND");
        return;
    }

    snprintf(
        out_frame->lines[0],
        C5_SCREEN_LINE_LEN,
        "MODE: %s",
        echoforest_mode_to_wire(state->mode)
    );
    snprintf(
        out_frame->lines[1],
        C5_SCREEN_LINE_LEN,
        "FOREST: %s",
        echoforest_forest_state_to_wire(state->forest_state)
    );
    snprintf(
        out_frame->lines[2],
        C5_SCREEN_LINE_LEN,
        "LIGHT/NOISE: %.0f/%.0f",
        state->environment.light,
        state->environment.noise
    );
    snprintf(
        out_frame->lines[3],
        C5_SCREEN_LINE_LEN,
        "TEMP/HUMI: %.1fC/%.0f%%",
        state->environment.temp,
        state->environment.humi
    );
    snprintf(
        out_frame->lines[4],
        C5_SCREEN_LINE_LEN,
        "BACKEND:%s S3:%s",
        state->backend_online ? "ONLINE" : "OFFLINE",
        state->s3_online ? "ONLINE" : "OFFLINE"
    );
    snprintf(
        out_frame->lines[5],
        C5_SCREEN_LINE_LEN,
        "V%lu %s",
        state->version,
        echoforest_forest_state_to_wire(state->forest_state)
    );
}
