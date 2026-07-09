#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "c5_app.h"
#include "c5_bitmap_text_renderer.h"
#include "mock_cloud_transport.h"
#include "s3_app.h"

static int g_fill_rect_count = 0;

static void test_fill_rect(
    int x,
    int y,
    int width,
    int height,
    uint16_t color,
    void *user_ctx
) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)color;
    (void)user_ctx;
    g_fill_rect_count += 1;
}

static void test_protocol_and_touch(void) {
    assert(echoforest_mode_from_wire("focus") == ECHOFOREST_MODE_FOCUS);
    assert(echoforest_forest_state_from_wire("noise_warning") == ECHOFOREST_FOREST_NOISE_WARNING);
    assert(strcmp(echoforest_control_action_to_path(ECHOFOREST_CONTROL_END_FOCUS), "/api/control/end_focus") == 0);

    c5_touch_layout_t layout = c5_default_touch_layout(240, 240);
    assert(c5_touch_point_to_zone(layout.start.x + 1, layout.start.y + 1, &layout) == C5_TOUCH_ZONE_START);
    assert(c5_touch_point_to_zone(layout.break_button.x + 1, layout.break_button.y + 1, &layout) == C5_TOUCH_ZONE_BREAK);
    assert(c5_touch_point_to_zone(layout.end.x + 1, layout.end.y + 1, &layout) == C5_TOUCH_ZONE_END);
    assert(c5_touch_point_to_zone(0, 0, &layout) == C5_TOUCH_ZONE_NONE);
}

static void test_c5_app_with_mock_transport(void) {
    echoforest_mock_transport_reset();

    echoforest_cloud_config_t config = {
        .backend_base_url = "mock://backend",
        .device_id = "c5-control",
        .device_role = "c5",
    };

    c5_app_init(&config, echoforest_mock_transport());
    assert(echoforest_mock_transport_heartbeat_count() == 1);

    c5_touch_layout_t layout = c5_default_touch_layout(240, 240);
    assert(c5_app_handle_touch_point(layout.start.x + 2, layout.start.y + 2, &layout, "host test") == 0);
    assert(echoforest_mock_transport_last_action() == ECHOFOREST_CONTROL_START_FOCUS);
    assert(echoforest_mock_transport_control_count() == 1);

    c5_screen_frame_t frame;
    c5_speaker_prompt_t prompt;
    assert(c5_app_poll_backend(&frame, &prompt) == 0);
    assert(strstr(frame.lines[0], "MODE:") != 0);
    assert(strstr(frame.lines[1], "FOREST:") != 0);
}

static void test_s3_app_with_mock_transport(void) {
    echoforest_mock_transport_reset();

    echoforest_cloud_config_t config = {
        .backend_base_url = "mock://backend",
        .device_id = "s3-main",
        .device_role = "s3",
    };

    s3_app_init(&config, echoforest_mock_transport(), S3_SENSOR_SOURCE_MOCK);
    assert(echoforest_mock_transport_heartbeat_count() == 1);

    s3_screen_frame_t frame;
    assert(s3_app_sample_upload_and_refresh(&frame) == 0);
    assert(echoforest_mock_transport_environment_count() == 1);
    assert(strstr(frame.lines[2], "light") != 0);
}

static void test_bitmap_renderer(void) {
    c5_screen_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    snprintf(frame.lines[0], sizeof(frame.lines[0]), "MODE: FOCUS");
    snprintf(frame.lines[1], sizeof(frame.lines[1]), "FOREST: GROWING");

    c5_bitmap_text_config_t config = {
        .fill_rect = test_fill_rect,
        .user_ctx = 0,
        .fg_color = 0xffff,
        .bg_color = 0x0000,
        .scale = 1,
        .char_spacing = 1,
        .line_spacing = 2,
    };

    g_fill_rect_count = 0;
    assert(c5_bitmap_draw_frame(&config, 0, 0, &frame) == 0);
    assert(g_fill_rect_count > 0);
}

int main(void) {
    test_protocol_and_touch();
    test_c5_app_with_mock_transport();
    test_s3_app_with_mock_transport();
    test_bitmap_renderer();
    puts("firmware business logic tests passed");
    return 0;
}
