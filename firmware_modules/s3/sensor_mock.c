#include "sensor_mock.h"

#include <string.h>

static int g_sample_index = 0;

void s3_sensor_mock_reset(void) {
    g_sample_index = 0;
}

int s3_sensor_mock_read(echoforest_environment_t *out_environment) {
    if (out_environment == 0) {
        return -1;
    }

    static const echoforest_environment_t samples[] = {
        { .light = 320.0f, .noise = 38.0f, .temp = 24.0f, .humi = 50.0f },
        { .light = 90.0f,  .noise = 39.0f, .temp = 24.2f, .humi = 51.0f },
        { .light = 260.0f, .noise = 78.0f, .temp = 24.4f, .humi = 52.0f },
        { .light = 280.0f, .noise = 42.0f, .temp = 31.0f, .humi = 74.0f },
        { .light = 300.0f, .noise = 41.0f, .temp = 24.5f, .humi = 50.0f },
    };

    const int count = (int)(sizeof(samples) / sizeof(samples[0]));
    *out_environment = samples[g_sample_index % count];
    out_environment->updated_at[0] = '\0';
    g_sample_index += 1;
    return 0;
}
