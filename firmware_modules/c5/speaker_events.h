#pragma once

#include "../common/echoforest_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int should_prompt;
    const char *prompt_text;
} c5_speaker_prompt_t;

c5_speaker_prompt_t c5_speaker_prompt_for_transition(
    echoforest_forest_state_t previous_state,
    echoforest_forest_state_t current_state
);

#ifdef __cplusplus
}
#endif
