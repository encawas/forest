#pragma once

#include "echoforest_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *backend_base_url;
    const char *device_id;
    const char *device_role;
} echoforest_cloud_config_t;

typedef struct {
    int (*post_control)(echoforest_control_action_t action, const char *source, const char *goal);
    int (*post_environment)(const char *device_id, const char *source, const echoforest_environment_t *environment);
    int (*post_heartbeat)(const char *device_id, const char *role, const char *source);
    int (*get_state)(echoforest_state_snapshot_t *out_state);
} echoforest_cloud_transport_t;

void echoforest_cloud_client_init(
    const echoforest_cloud_config_t *config,
    const echoforest_cloud_transport_t *transport
);

int echoforest_cloud_post_control(echoforest_control_action_t action, const char *goal);
int echoforest_cloud_post_environment(const echoforest_environment_t *environment);
int echoforest_cloud_post_heartbeat(void);
int echoforest_cloud_get_state(echoforest_state_snapshot_t *out_state);

#ifdef __cplusplus
}
#endif
