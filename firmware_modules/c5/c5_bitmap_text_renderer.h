#pragma once

#include <stdint.h>

#include "control_screen_renderer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*c5_bitmap_fill_rect_fn)(
    int x,
    int y,
    int width,
    int height,
    uint16_t color,
    void *user_ctx
);

typedef struct {
    c5_bitmap_fill_rect_fn fill_rect;
    void *user_ctx;
    uint16_t fg_color;
    uint16_t bg_color;
    int scale;
    int char_spacing;
    int line_spacing;
} c5_bitmap_text_config_t;

int c5_bitmap_draw_text(
    const c5_bitmap_text_config_t *config,
    int x,
    int y,
    const char *text
);

int c5_bitmap_draw_frame(
    const c5_bitmap_text_config_t *config,
    int x,
    int y,
    const c5_screen_frame_t *frame
);

#ifdef __cplusplus
}
#endif
