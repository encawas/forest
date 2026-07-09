idf_component_register(
    SRCS
        "s3_app.c"
        "main_screen_renderer.c"
        "sensor_mock.c"
        "sensor_real_adapter.c"
    INCLUDE_DIRS "." "../common"
    REQUIRES common
)
