#include "esp_http_transport.h"

#include <stdio.h>
#include <string.h>

#include "cJSON.h"
#include "esp_http_client.h"
#include "esp_log.h"

#define ECHOFOREST_HTTP_URL_LEN 192
#define ECHOFOREST_HTTP_BODY_LEN 512
#define ECHOFOREST_HTTP_RESPONSE_LEN 4096

static const char *TAG = "echoforest_http";
static char g_backend_base_url[ECHOFOREST_HTTP_URL_LEN] = "http://127.0.0.1:3001";

static void safe_copy(char *dst, size_t dst_len, const char *src) {
    if (dst == 0 || dst_len == 0) return;
    if (src == 0) {
        dst[0] = '\0';
        return;
    }
    snprintf(dst, dst_len, "%s", src);
}

static const cJSON *json_get_object(const cJSON *root, const char *name) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(root, name);
    return cJSON_IsObject(item) ? item : 0;
}

static const char *json_get_string(const cJSON *root, const char *name) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(root, name);
    return cJSON_IsString(item) ? item->valuestring : 0;
}

static double json_get_number(const cJSON *root, const char *name, double fallback) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(root, name);
    return cJSON_IsNumber(item) ? item->valuedouble : fallback;
}

static int json_get_bool(const cJSON *root, const char *name, int fallback) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(root, name);
    if (cJSON_IsBool(item)) return cJSON_IsTrue(item) ? 1 : 0;
    return fallback;
}

static int build_url(char *out_url, size_t out_len, const char *path) {
    if (out_url == 0 || out_len == 0 || path == 0) return -1;
    int written = snprintf(out_url, out_len, "%s%s", g_backend_base_url, path);
    return (written > 0 && (size_t)written < out_len) ? 0 : -1;
}

static int http_request(
    esp_http_client_method_t method,
    const char *path,
    const char *body,
    char *response,
    size_t response_len
) {
    char url[ECHOFOREST_HTTP_URL_LEN];
    if (build_url(url, sizeof(url), path) != 0) {
        ESP_LOGE(TAG, "URL too long: %s%s", g_backend_base_url, path);
        return -1;
    }

    esp_http_client_config_t config = {
        .url = url,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == 0) {
        return -1;
    }

    esp_http_client_set_method(client, method);

    if (body != 0) {
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, body, (int)strlen(body));
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return -1;
    }

    int status = esp_http_client_get_status_code(client);
    if (response != 0 && response_len > 0) {
        int read_len = esp_http_client_read_response(client, response, (int)response_len - 1);
        if (read_len < 0) read_len = 0;
        response[read_len] = '\0';
    }

    esp_http_client_cleanup(client);
    return (status >= 200 && status < 300) ? 0 : -1;
}

static void parse_environment(const cJSON *root, echoforest_state_snapshot_t *out_state) {
    const cJSON *environment = json_get_object(root, "environment");
    if (environment == 0) return;

    out_state->environment.light = (float)json_get_number(environment, "light", 0);
    out_state->environment.noise = (float)json_get_number(environment, "noise", 0);
    out_state->environment.temp = (float)json_get_number(environment, "temp", 0);
    out_state->environment.humi = (float)json_get_number(environment, "humi", 0);
    safe_copy(
        out_state->environment.updated_at,
        sizeof(out_state->environment.updated_at),
        json_get_string(environment, "updatedAt")
    );
}

static void parse_session(const cJSON *root, echoforest_state_snapshot_t *out_state) {
    const cJSON *session = json_get_object(root, "session");
    if (session == 0) {
        out_state->goal[0] = '\0';
        return;
    }
    safe_copy(out_state->goal, sizeof(out_state->goal), json_get_string(session, "goal"));
}

static void parse_devices(const cJSON *root, echoforest_state_snapshot_t *out_state) {
    const cJSON *devices = json_get_object(root, "devices");
    if (devices == 0) return;

    const cJSON *device = 0;
    cJSON_ArrayForEach(device, devices) {
        const char *role = json_get_string(device, "role");
        int online = json_get_bool(device, "online", 0);
        if (role != 0 && strcmp(role, "s3") == 0) {
            out_state->s3_online = online;
        }
    }
}

static int parse_state_response(const char *json_text, echoforest_state_snapshot_t *out_state) {
    if (json_text == 0 || out_state == 0) return -1;

    cJSON *root = cJSON_Parse(json_text);
    if (root == 0) {
        ESP_LOGE(TAG, "Failed to parse backend JSON state");
        return -1;
    }

    memset(out_state, 0, sizeof(*out_state));
    out_state->version = (unsigned long)json_get_number(root, "version", 0);
    out_state->mode = echoforest_mode_from_wire(json_get_string(root, "mode"));
    out_state->forest_state = echoforest_forest_state_from_wire(json_get_string(root, "forest_state"));
    out_state->backend_online = 1;
    safe_copy(out_state->reason, sizeof(out_state->reason), json_get_string(root, "reason"));
    parse_environment(root, out_state);
    parse_session(root, out_state);
    parse_devices(root, out_state);

    cJSON_Delete(root);
    return 0;
}

static int esp_post_control(echoforest_control_action_t action, const char *source, const char *goal) {
    char body[ECHOFOREST_HTTP_BODY_LEN];
    snprintf(
        body,
        sizeof(body),
        "{\"source\":\"%s\",\"goal\":\"%s\"}",
        source != 0 ? source : "device",
        goal != 0 ? goal : ""
    );
    return http_request(HTTP_METHOD_POST, echoforest_control_action_to_path(action), body, 0, 0);
}

static int esp_post_environment(
    const char *device_id,
    const char *source,
    const echoforest_environment_t *environment
) {
    if (environment == 0) return -1;

    char body[ECHOFOREST_HTTP_BODY_LEN];
    snprintf(
        body,
        sizeof(body),
        "{\"deviceId\":\"%s\",\"source\":\"%s\",\"light\":%.2f,\"noise\":%.2f,\"temp\":%.2f,\"humi\":%.2f}",
        device_id != 0 ? device_id : "s3-main",
        source != 0 ? source : "s3",
        environment->light,
        environment->noise,
        environment->temp,
        environment->humi
    );
    return http_request(HTTP_METHOD_POST, "/api/device_data", body, 0, 0);
}

static int esp_post_heartbeat(const char *device_id, const char *role, const char *source) {
    char body[ECHOFOREST_HTTP_BODY_LEN];
    snprintf(
        body,
        sizeof(body),
        "{\"deviceId\":\"%s\",\"role\":\"%s\",\"source\":\"%s\"}",
        device_id != 0 ? device_id : "unknown-device",
        role != 0 ? role : "unknown",
        source != 0 ? source : "unknown"
    );
    return http_request(HTTP_METHOD_POST, "/api/devices/heartbeat", body, 0, 0);
}

static int esp_get_state(echoforest_state_snapshot_t *out_state) {
    char response[ECHOFOREST_HTTP_RESPONSE_LEN];
    if (http_request(HTTP_METHOD_GET, "/api/state", 0, response, sizeof(response)) != 0) {
        return -1;
    }
    return parse_state_response(response, out_state);
}

static const echoforest_cloud_transport_t g_transport = {
    .post_control = esp_post_control,
    .post_environment = esp_post_environment,
    .post_heartbeat = esp_post_heartbeat,
    .get_state = esp_get_state,
};

void echoforest_esp_http_transport_init(const char *backend_base_url) {
    if (backend_base_url != 0 && backend_base_url[0] != '\0') {
        safe_copy(g_backend_base_url, sizeof(g_backend_base_url), backend_base_url);
    }
}

const echoforest_cloud_transport_t *echoforest_esp_http_transport(void) {
    return &g_transport;
}
