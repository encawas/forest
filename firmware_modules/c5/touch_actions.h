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
    int has_action;
    echoforest_control_action_t action;
} c5_touch_action_result_t;

c5_touch_action_result_t c5_touch_zone_to_action(c5_touch_zone_t zone);

#ifdef __cplusplus
}
#endif
