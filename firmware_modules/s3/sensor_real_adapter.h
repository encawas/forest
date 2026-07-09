#pragma once

#include "../common/echoforest_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int (*read_light)(float *out_light);
    int (*read_noise)(float *out_noise);
    int (*read_temp)(float *out_temp);
    int (*read_humi)(float *out_humi);
} s3_sensor_driver_t;

void s3_sensor_real_adapter_init(const s3_sensor_driver_t *driver);
int s3_sensor_real_read(echoforest_environment_t *out_environment);

#ifdef __cplusplus
}
#endif
