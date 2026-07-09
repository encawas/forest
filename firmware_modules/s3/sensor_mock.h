#pragma once

#include "../common/echoforest_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

void s3_sensor_mock_reset(void);
int s3_sensor_mock_read(echoforest_environment_t *out_environment);

#ifdef __cplusplus
}
#endif
