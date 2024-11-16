
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} DIR_INTERFACE_SRCS)

add_library(lpa_js STATIC ${LIB_CJSON_SRCS}
        main.c
        lpa_info.c
        lpa_profiles.c
        lpa_notification.c
        functions.h
        main.h
        state_serializer.c
        state_serializer.h)
target_include_directories(lpa_js PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)
