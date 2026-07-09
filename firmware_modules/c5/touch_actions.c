#include "touch_actions.h"

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
