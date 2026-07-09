#include "main_screen_renderer.h"

#include <stdio.h>

static const char *forest_title(echoforest_forest_state_t state) {
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

static const char *forest_hint(echoforest_forest_state_t state) {
    switch (state) {
        case ECHOFOREST_FOREST_GROWING:
            return "保持当前环境，森林正在成长";
        case ECHOFOREST_FOREST_LIGHT_WARNING:
            return "请提高照明，保护眼睛";
        case ECHOFOREST_FOREST_NOISE_WARNING:
            return "请降低噪声，保持专注";
        case ECHOFOREST_FOREST_COMFORT_WARNING:
            return "请调整温湿度，让环境更舒适";
        case ECHOFOREST_FOREST_BREAK:
            return "短暂休息，稍后继续";
        case ECHOFOREST_FOREST_COMPLETED:
            return "本轮学习已完成";
        case ECHOFOREST_FOREST_IDLE:
        default:
            return "等待网页或 C5 开始学习";
    }
}

static void clear_frame(s3_screen_frame_t *frame) {
    for (int i = 0; i < S3_SCREEN_LINE_COUNT; ++i) {
        frame->lines[i][0] = '\0';
    }
}

void s3_render_main_screen(
    const echoforest_state_snapshot_t *state,
    s3_screen_frame_t *out_frame
) {
    if (out_frame == 0) {
        return;
    }

    clear_frame(out_frame);

    if (state == 0) {
        snprintf(out_frame->lines[0], S3_SCREEN_LINE_LEN, "EchoForest S3");
        snprintf(out_frame->lines[1], S3_SCREEN_LINE_LEN, "等待后端状态...");
        return;
    }

    snprintf(out_frame->lines[0], S3_SCREEN_LINE_LEN, "%s", forest_title(state->forest_state));
    snprintf(out_frame->lines[1], S3_SCREEN_LINE_LEN, "%s", forest_hint(state->forest_state));
    snprintf(
        out_frame->lines[2],
        S3_SCREEN_LINE_LEN,
        "light %.0f | noise %.0f",
        state->environment.light,
        state->environment.noise
    );
    snprintf(
        out_frame->lines[3],
        S3_SCREEN_LINE_LEN,
        "temp %.1fC | humi %.0f%%",
        state->environment.temp,
        state->environment.humi
    );
    snprintf(
        out_frame->lines[4],
        S3_SCREEN_LINE_LEN,
        "backend %s | v%lu",
        state->backend_online ? "online" : "offline",
        state->version
    );
}
