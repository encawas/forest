#pragma once

#include "../common/echoforest_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

#define S3_SCREEN_LINE_COUNT 5
#define S3_SCREEN_LINE_LEN 128

typedef struct {
    char lines[S3_SCREEN_LINE_COUNT][S3_SCREEN_LINE_LEN];
} s3_screen_frame_t;

void s3_render_main_screen(
    const echoforest_state_snapshot_t *state,
    s3_screen_frame_t *out_frame
);

#ifdef __cplusplus
}
#endif
