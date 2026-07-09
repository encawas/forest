idf_component_register(
    SRCS
        "c5_app.c"
        "control_screen_renderer.c"
        "speaker_events.c"
        "touch_actions.c"
        "c5_bitmap_text_renderer.c"
    INCLUDE_DIRS "." "../common"
    REQUIRES common
)
