#pragma once

#include "cloud_client.h"

#ifdef __cplusplus
extern "C" {
#endif

void echoforest_mock_transport_reset(void);
void echoforest_mock_transport_set_state(const echoforest_state_snapshot_t *state);
const echoforest_cloud_transport_t *echoforest_mock_transport(void);

echoforest_control_action_t echoforest_mock_transport_last_action(void);
const echoforest_environment_t *echoforest_mock_transport_last_environment(void);
int echoforest_mock_transport_control_count(void);
int echoforest_mock_transport_environment_count(void);
int echoforest_mock_transport_heartbeat_count(void);

#ifdef __cplusplus
}
#endif
