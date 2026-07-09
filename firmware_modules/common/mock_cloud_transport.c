#include "mock_cloud_transport.h"

#include <string.h>

static echoforest_state_snapshot_t g_state;
static echoforest_control_action_t g_last_action = ECHOFOREST_CONTROL_START_FOCUS;
static echoforest_environment_t g_last_environment;
static int g_control_count = 0;
static int g_environment_count = 0;
static int g_heartbeat_count = 0;

void echoforest_mock_transport_reset(void) {
    memset(&g_state, 0, sizeof(g_state));
    memset(&g_last_environment, 0, sizeof(g_last_environment));
    g_state.mode = ECHOFOREST_MODE_IDLE;
    g_state.forest_state = ECHOFOREST_FOREST_IDLE;
    g_state.backend_online = 1;
    g_last_action = ECHOFOREST_CONTROL_START_FOCUS;
    g_control_count = 0;
    g_environment_count = 0;
    g_heartbeat_count = 0;
}

void echoforest_mock_transport_set_state(const echoforest_state_snapshot_t *state) {
    if (state != 0) {
        g_state = *state;
    }
}

static int mock_post_control(echoforest_control_action_t action, const char *source, const char *goal) {
    (void)source;
    g_last_action = action;
    g_control_count += 1;

    if (action == ECHOFOREST_CONTROL_START_FOCUS) {
        g_state.mode = ECHOFOREST_MODE_FOCUS;
        g_state.forest_state = ECHOFOREST_FOREST_GROWING;
        if (goal != 0) {
            snprintf(g_state.goal, sizeof(g_state.goal), "%s", goal);
        }
    } else if (action == ECHOFOREST_CONTROL_START_BREAK) {
        g_state.mode = ECHOFOREST_MODE_BREAK;
        g_state.forest_state = ECHOFOREST_FOREST_BREAK;
    } else if (action == ECHOFOREST_CONTROL_END_FOCUS) {
        g_state.mode = ECHOFOREST_MODE_COMPLETED;
        g_state.forest_state = ECHOFOREST_FOREST_COMPLETED;
    }

    g_state.version += 1;
    return 0;
}

static int mock_post_environment(
    const char *device_id,
    const char *source,
    const echoforest_environment_t *environment
) {
    (void)device_id;
    (void)source;
    if (environment == 0) {
        return -1;
    }
    g_last_environment = *environment;
    g_state.environment = *environment;
    g_state.s3_online = 1;
    g_state.version += 1;
    g_environment_count += 1;
    return 0;
}

static int mock_post_heartbeat(const char *device_id, const char *role, const char *source) {
    (void)device_id;
    (void)source;
    if (role != 0 && strcmp(role, "s3") == 0) {
        g_state.s3_online = 1;
    }
    g_heartbeat_count += 1;
    return 0;
}

static int mock_get_state(echoforest_state_snapshot_t *out_state) {
    if (out_state == 0) {
        return -1;
    }
    *out_state = g_state;
    out_state->backend_online = 1;
    return 0;
}

static const echoforest_cloud_transport_t g_transport = {
    .post_control = mock_post_control,
    .post_environment = mock_post_environment,
    .post_heartbeat = mock_post_heartbeat,
    .get_state = mock_get_state,
};

const echoforest_cloud_transport_t *echoforest_mock_transport(void) {
    return &g_transport;
}

echoforest_control_action_t echoforest_mock_transport_last_action(void) {
    return g_last_action;
}

const echoforest_environment_t *echoforest_mock_transport_last_environment(void) {
    return &g_last_environment;
}

int echoforest_mock_transport_control_count(void) {
    return g_control_count;
}

int echoforest_mock_transport_environment_count(void) {
    return g_environment_count;
}

int echoforest_mock_transport_heartbeat_count(void) {
    return g_heartbeat_count;
}
