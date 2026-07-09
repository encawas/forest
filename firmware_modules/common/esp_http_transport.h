#pragma once

#include "cloud_client.h"

#ifdef __cplusplus
extern "C" {
#endif

void echoforest_esp_http_transport_init(const char *backend_base_url);
const echoforest_cloud_transport_t *echoforest_esp_http_transport(void);

#ifdef __cplusplus
}
#endif
