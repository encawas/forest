#include "touch_actions.h"

static int point_in_rect(int x, int y, const c5_touch_rect_t *rect) {
    if (rect == 0) {
        return 0;
    }
    return x >= rect->x
        && y >= rect->y
        && x < rect->x + rect->width
        && y < rect->y + rect->height;
}

c5_touch_layout_t c5_default_touch_layout(int screen_width, int screen_height) {
    c5_touch_layout_t layout;

    int margin = screen_width / 20;
    if (margin < 8) margin = 8;

    int gap = margin;
    int button_y = screen_height * 3 / 4;
    int button_h = screen_height / 6;
    if (button_h < 28) button_h = 28;

    int button_w = (screen_width - margin * 2 - gap * 2) / 3;
    if (button_w < 30) button_w = 30;

    layout.start.x = margin;
    layout.start.y = button_y;
    layout.start.width = button_w;
    layout.start.height = button_h;

    layout.break_button.x = margin + button_w + gap;
    layout.break_button.y = button_y;
    layout.break_button.width = button_w;
    layout.break_button.height = button_h;

    layout.end.x = margin + (button_w + gap) * 2;
    layout.end.y = button_y;
    layout.end.width = button_w;
    layout.end.height = button_h;

    return layout;
}

c5_touch_zone_t c5_touch_point_to_zone(int x, int y, const c5_touch_layout_t *layout) {
    if (layout == 0) {
        return C5_TOUCH_ZONE_NONE;
    }

    if (point_in_rect(x, y, &layout->start)) {
        return C5_TOUCH_ZONE_START;
    }
    if (point_in_rect(x, y, &layout->break_button)) {
        return C5_TOUCH_ZONE_BREAK;
    }
    if (point_in_rect(x, y, &layout->end)) {
        return C5_TOUCH_ZONE_END;
    }
    return C5_TOUCH_ZONE_NONE;
}

c5_touch_action_result_t c5_touch_zone_to_action(c5_touch_zone_t zone) {
    c5_touch_action_result_t result;
    result.has_action = 0;
    result.action = ECHOFOREST_CONTROL_START_FOCUS;

    switch (zone) {
        case C5_TOUCH_ZONE_START:
            result.has_action = 1;
            result.action = ECHOFOREST_CONTROL_START_FOCUS;
            break;
        case C5_TOUCH_ZONE_BREAK:
            result.has_action = 1;
            result.action = ECHOFOREST_CONTROL_START_BREAK;
            break;
        case C5_TOUCH_ZONE_END:
            result.has_action = 1;
            result.action = ECHOFOREST_CONTROL_END_FOCUS;
            break;
        case C5_TOUCH_ZONE_NONE:
        default:
            break;
    }

    return result;
}

c5_touch_action_result_t c5_touch_point_to_action(int x, int y, const c5_touch_layout_t *layout) {
    return c5_touch_zone_to_action(c5_touch_point_to_zone(x, y, layout));
}
