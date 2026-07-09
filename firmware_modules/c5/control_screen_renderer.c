#include "control_screen_renderer.h"

#include <stdio.h>
#include <string.h>

static const char *mode_label(echoforest_mode_t mode) {
    switch (mode) {
        case ECHOFOREST_MODE_FOCUS:
            return "学习中";
        case ECHOFOREST_MODE_BREAK:
            return "休息中";
        case ECHOFOREST_MODE_COMPLETED:
            return "已完成";
        case ECHOFOREST_MODE_IDLE:
        default:
            return "待开始";
    }
}

static const char *forest_label(echoforest_forest_state_t state) {
    switch (state) {
        case ECHOFOREST_FOREST_GROWING:
            return "森林成长中";
        case ECHOFOREST_FOREST_LIGHT_WARNING:
            return "光线提醒";
        case ECHOFOREST_FOREST_NOISE_WARNING:
            return "噪声提醒";
        case ECHOFOREST_FOREST_COMFORT_WARNING:
            return "舒适度提醒";
        case ECHOFOREST_FOREST_BREAK:
            return "休息中";
        case ECHOFOREST_FOREST_COMPLETED:
            return "学习完成";
        case ECHOFOREST_FOREST_IDLE:
        default:
            return "等待学习";
    }
}

static void clear_frame(c5_screen_frame_t *frame) {
    for (int i = 0; i < C5_SCREEN_LINE_COUNT; ++i) {
        frame->lines[i][0] = '\0';
    }
}

void c5_render_control_screen(
    const echoforest_state_snapshot_t *state,
    c5_screen_frame_t *out_frame
) {
    if (out_frame == NULL) {
        return;
    }

    clear_frame(out_frame);

    if (state == NULL) {
        snprintf(out_frame->lines[0], C5_SCREEN_LINE_LEN, "EchoForest C5");
        snprintf(out_frame->lines[1], C5_SCREEN_LINE_LEN, "等待后端状态...");
        return;
    }

    snprintf(out_frame->lines[0], C5_SCREEN_LINE_LEN, "模式: %s", mode_label(state->mode));
    snprintf(out_frame->lines[1], C5_SCREEN_LINE_LEN, "森林: %s", forest_label(state->forest_state));
    snprintf(
        out_frame->lines[2],
        C5_SCREEN_LINE_LEN,
        "光/噪: %.0f / %.0f",
        state->environment.light,
        state->environment.noise
    );
    snprintf(
        out_frame->lines[3],
        C5_SCREEN_LINE_LEN,
        "温/湿: %.1fC / %.0f%%",
        state->environment.temp,
        state->environment.humi
    );
    snprintf(
        out_frame->lines[4],
        C5_SCREEN_LINE_LEN,
        "后端:%s S3:%s",
        state->backend_online ? "在线" : "离线",
        state->s3_online ? "在线" : "离线"
    );
    snprintf(out_frame->lines[5], C5_SCREEN_LINE_LEN, "v%lu %s", state->version, state->reason);
}
