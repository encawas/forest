#pragma once

#include "../common/echoforest_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

#define C5_SCREEN_LINE_COUNT 6
#define C5_SCREEN_LINE_LEN 128

typedef struct {
    char lines[C5_SCREEN_LINE_COUNT][C5_SCREEN_LINE_LEN];
} c5_screen_frame_t;

void c5_render_control_screen(
    const echoforest_state_snapshot_t *state,
    c5_screen_frame_t *out_frame
);

#ifdef __cplusplus
}
#endif
