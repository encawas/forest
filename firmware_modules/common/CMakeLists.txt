set(ECHOFOREST_COMMON_SRCS
    "cloud_client.c"
    "echoforest_protocol.c"
    "mock_cloud_transport.c"
)

if(ESP_PLATFORM)
    list(APPEND ECHOFOREST_COMMON_SRCS "esp_http_transport.c")
endif()

idf_component_register(
    SRCS ${ECHOFOREST_COMMON_SRCS}
    INCLUDE_DIRS "."
    REQUIRES esp_http_client json
)
