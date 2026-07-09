#include "echoforest_protocol.h"

#include <string.h>

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

echoforest_mode_t echoforest_mode_from_wire(const char *mode) {
    if (mode == 0) return ECHOFOREST_MODE_IDLE;
    if (strcmp(mode, "focus") == 0) return ECHOFOREST_MODE_FOCUS;
    if (strcmp(mode, "break") == 0) return ECHOFOREST_MODE_BREAK;
    if (strcmp(mode, "completed") == 0) return ECHOFOREST_MODE_COMPLETED;
    return ECHOFOREST_MODE_IDLE;
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

echoforest_forest_state_t echoforest_forest_state_from_wire(const char *state) {
    if (state == 0) return ECHOFOREST_FOREST_IDLE;
    if (strcmp(state, "growing") == 0) return ECHOFOREST_FOREST_GROWING;
    if (strcmp(state, "light_warning") == 0) return ECHOFOREST_FOREST_LIGHT_WARNING;
    if (strcmp(state, "noise_warning") == 0) return ECHOFOREST_FOREST_NOISE_WARNING;
    if (strcmp(state, "comfort_warning") == 0) return ECHOFOREST_FOREST_COMFORT_WARNING;
    if (strcmp(state, "break") == 0) return ECHOFOREST_FOREST_BREAK;
    if (strcmp(state, "completed") == 0) return ECHOFOREST_FOREST_COMPLETED;
    return ECHOFOREST_FOREST_IDLE;
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
