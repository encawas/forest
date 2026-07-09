#pragma once

#include "../common/echoforest_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    C5_TOUCH_ZONE_NONE = 0,
    C5_TOUCH_ZONE_START,
    C5_TOUCH_ZONE_BREAK,
    C5_TOUCH_ZONE_END,
} c5_touch_zone_t;

typedef struct {
    int x;
    int y;
    int width;
    int height;
} c5_touch_rect_t;

typedef struct {
    c5_touch_rect_t start;
    c5_touch_rect_t break_button;
    c5_touch_rect_t end;
} c5_touch_layout_t;

typedef struct {
    int has_action;
    echoforest_control_action_t action;
} c5_touch_action_result_t;

c5_touch_layout_t c5_default_touch_layout(int screen_width, int screen_height);
c5_touch_zone_t c5_touch_point_to_zone(int x, int y, const c5_touch_layout_t *layout);
c5_touch_action_result_t c5_touch_zone_to_action(c5_touch_zone_t zone);
c5_touch_action_result_t c5_touch_point_to_action(int x, int y, const c5_touch_layout_t *layout);

#ifdef __cplusplus
}
#endif
