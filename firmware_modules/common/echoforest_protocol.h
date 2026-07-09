#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define ECHOFOREST_MAX_TEXT_LEN 96
#define ECHOFOREST_DEVICE_ID_LEN 32

typedef enum {
    ECHOFOREST_MODE_IDLE = 0,
    ECHOFOREST_MODE_FOCUS,
    ECHOFOREST_MODE_BREAK,
    ECHOFOREST_MODE_COMPLETED,
} echoforest_mode_t;

typedef enum {
    ECHOFOREST_FOREST_IDLE = 0,
    ECHOFOREST_FOREST_GROWING,
    ECHOFOREST_FOREST_LIGHT_WARNING,
    ECHOFOREST_FOREST_NOISE_WARNING,
    ECHOFOREST_FOREST_COMFORT_WARNING,
    ECHOFOREST_FOREST_BREAK,
    ECHOFOREST_FOREST_COMPLETED,
} echoforest_forest_state_t;

typedef enum {
    ECHOFOREST_CONTROL_START_FOCUS = 0,
    ECHOFOREST_CONTROL_START_BREAK,
    ECHOFOREST_CONTROL_END_FOCUS,
} echoforest_control_action_t;

typedef struct {
    float light;
    float noise;
    float temp;
    float humi;
    char updated_at[ECHOFOREST_MAX_TEXT_LEN];
} echoforest_environment_t;

typedef struct {
    char id[ECHOFOREST_DEVICE_ID_LEN];
    const char *role;
    int online;
    char last_seen_at[ECHOFOREST_MAX_TEXT_LEN];
} echoforest_device_status_t;

typedef struct {
    unsigned long version;
    echoforest_mode_t mode;
    echoforest_forest_state_t forest_state;
    echoforest_environment_t environment;
    char reason[ECHOFOREST_MAX_TEXT_LEN];
    char goal[ECHOFOREST_MAX_TEXT_LEN];
    int s3_online;
    int backend_online;
} echoforest_state_snapshot_t;

const char *echoforest_mode_to_wire(echoforest_mode_t mode);
echoforest_mode_t echoforest_mode_from_wire(const char *mode);
const char *echoforest_forest_state_to_wire(echoforest_forest_state_t state);
echoforest_forest_state_t echoforest_forest_state_from_wire(const char *state);
const char *echoforest_control_action_to_path(echoforest_control_action_t action);

#ifdef __cplusplus
}
#endif
