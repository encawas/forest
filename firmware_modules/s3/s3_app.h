#pragma once

#include "../common/cloud_client.h"
#include "main_screen_renderer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    S3_SENSOR_SOURCE_MOCK = 0,
    S3_SENSOR_SOURCE_REAL,
} s3_sensor_source_t;

void s3_app_init(
    const echoforest_cloud_config_t *cloud_config,
    const echoforest_cloud_transport_t *transport,
    s3_sensor_source_t sensor_source
);

int s3_app_sample_upload_and_refresh(s3_screen_frame_t *out_frame);
const echoforest_state_snapshot_t *s3_app_cached_state(void);

#ifdef __cplusplus
}
#endif
