#include "cloud_client.h"

#include <stddef.h>

static echoforest_cloud_config_t g_config;
static echoforest_cloud_transport_t g_transport;

void echoforest_cloud_client_init(
    const echoforest_cloud_config_t *config,
    const echoforest_cloud_transport_t *transport
) {
    if (config != NULL) {
        g_config = *config;
    }
    if (transport != NULL) {
        g_transport = *transport;
    }
}

int echoforest_cloud_post_control(echoforest_control_action_t action, const char *goal) {
    if (g_transport.post_control == NULL) {
        return -1;
    }
    return g_transport.post_control(action, g_config.device_role, goal);
}

int echoforest_cloud_post_environment(const echoforest_environment_t *environment) {
    if (environment == NULL || g_transport.post_environment == NULL) {
        return -1;
    }
    return g_transport.post_environment(
        g_config.device_id,
        g_config.device_role,
        environment
    );
}

int echoforest_cloud_post_heartbeat(void) {
    if (g_transport.post_heartbeat == NULL) {
        return -1;
    }
    return g_transport.post_heartbeat(
        g_config.device_id,
        g_config.device_role,
        g_config.device_role
    );
}

int echoforest_cloud_get_state(echoforest_state_snapshot_t *out_state) {
    if (out_state == NULL || g_transport.get_state == NULL) {
        return -1;
    }
    return g_transport.get_state(out_state);
}
