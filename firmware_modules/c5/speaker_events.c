#include "speaker_events.h"

c5_speaker_prompt_t c5_speaker_prompt_for_transition(
    echoforest_forest_state_t previous_state,
    echoforest_forest_state_t current_state
) {
    c5_speaker_prompt_t result;
    result.should_prompt = 0;
    result.prompt_text = "";

    if (previous_state == current_state) {
        return result;
    }

    switch (current_state) {
        case ECHOFOREST_FOREST_GROWING:
            result.should_prompt = 1;
            result.prompt_text = "环境正常，森林正在成长";
            break;
        case ECHOFOREST_FOREST_LIGHT_WARNING:
            result.should_prompt = 1;
            result.prompt_text = "光线有点暗，建议打开灯";
            break;
        case ECHOFOREST_FOREST_NOISE_WARNING:
            result.should_prompt = 1;
            result.prompt_text = "环境有点吵，建议降低噪声";
            break;
        case ECHOFOREST_FOREST_COMFORT_WARNING:
            result.should_prompt = 1;
            result.prompt_text = "温湿度不太舒适，建议调整环境";
            break;
        case ECHOFOREST_FOREST_BREAK:
            result.should_prompt = 1;
            result.prompt_text = "进入休息模式";
            break;
        case ECHOFOREST_FOREST_COMPLETED:
            result.should_prompt = 1;
            result.prompt_text = "学习完成，已生成日志";
            break;
        case ECHOFOREST_FOREST_IDLE:
        default:
            break;
    }

    return result;
}
