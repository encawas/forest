#include "echoforest_protocol.h"

const char *echoforest_mode_to_wire(echoforest_mode_t mode) {
    switch (mode) {
        case ECHOFOREST_MODE_FOCUS:
            return "focus";
        case ECHOFOREST_MODE_BREAK:
            return "break";
        case ECHOFOREST_MODE_COMPLETED:
            return "completed";
        case ECHOFOREST_MODE_IDLE:
        default:
            return "idle";
    }
}

const char *echoforest_forest_state_to_wire(echoforest_forest_state_t state) {
    switch (state) {
        case ECHOFOREST_FOREST_GROWING:
            return "growing";
        case ECHOFOREST_FOREST_LIGHT_WARNING:
            return "light_warning";
        case ECHOFOREST_FOREST_NOISE_WARNING:
            return "noise_warning";
        case ECHOFOREST_FOREST_COMFORT_WARNING:
            return "comfort_warning";
        case ECHOFOREST_FOREST_BREAK:
            return "break";
        case ECHOFOREST_FOREST_COMPLETED:
            return "completed";
        case ECHOFOREST_FOREST_IDLE:
        default:
            return "idle";
    }
}

const char *echoforest_control_action_to_path(echoforest_control_action_t action) {
    switch (action) {
        case ECHOFOREST_CONTROL_START_FOCUS:
            return "/api/control/start_focus";
        case ECHOFOREST_CONTROL_START_BREAK:
            return "/api/control/start_break";
        case ECHOFOREST_CONTROL_END_FOCUS:
            return "/api/control/end_focus";
        default:
            return "/api/state";
    }
}
