cmake_minimum_required(VERSION 3.0.0)
project(rkaudio)

ADD_DEFINITIONS( -D__LINUX__)

SET (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(RKAUDIO_COMMON_SOURCE common/audio_misc.c
                          common/audio_queue.c
                          common/audio_stream.c
                          common/pcmout.c
                          common/platform_linux.c
                          common/capture_impl_alsa.c
                          common/playback_impl_alsa.c)

include_directories(./)
include_directories(./common)

if(RKAUDIO_RECORDER)
ADD_DEFINITIONS( -DAUDIO_ENABLE_RECORDER)
aux_source_directory(recorder RKAUDIO_RECORDER_SOURCE)
include_directories(./recorder)
endif()

if(RKAUDIO_PLAYER)
ADD_DEFINITIONS( -DAUDIO_ENABLE_PLAYER)
aux_source_directory(player RKAUDIO_PLAYER_SOURCE)
include_directories(./player)
endif()

set(RKAUDIO_SOURCE_FILE
    ${RKAUDIO_RECORDER_SOURCE}
    ${RKAUDIO_PLAYER_SOURCE}
    ${RKAUDIO_COMMON_SOURCE}
)

if(RKAUDIO_PLUGINS)
    if(RKAUDIO_PLUGIN_RESAMPLE)
        include_directories(plugins/ssrc)
        aux_source_directory(plugins/ssrc RKAUDIO_PLUGIN_RESAMPLE_SOURCE)
        set(RKAUDIO_SOURCE_FILE
            ${RKAUDIO_SOURCE_FILE}
            ${RKAUDIO_PLUGIN_RESAMPLE_SOURCE})
    endif()
endif()

if(RKAUDIO_ENDECODE)
    if(RKAUDIO_DECODER_MP3)
        ADD_DEFINITIONS( -DAUDIO_DECODER_MP3)
        include_directories(codec/decode/mp3)
        aux_source_directory(codec/decode/mp3 RKAUDIO_DECODER_MP3_SOURCE)
        set(RKAUDIO_SOURCE_FILE
            ${RKAUDIO_SOURCE_FILE}
            ${RKAUDIO_DECODER_MP3_SOURCE})
    endif()
endif()

ADD_LIBRARY (rkaudio SHARED ${RKAUDIO_SOURCE_FILE})
SET_TARGET_PROPERTIES (rkaudio PROPERTIES VERSION 1.0 SOVERSION 0)
link_libraries(rkaudio)
link_directories(${PROJECT_BINARY_DIR}/lib/)

install(TARGETS ${LIBRARY_NAME}
        LIBRARY DESTINATION "lib"
        PUBLIC_HEADER DESTINATION "audio_server.h")

if(RKAUDIO_TEST_DEMO)
    ADD_EXECUTABLE(rkaudio_play example/rkaudio_play.c)
    target_link_libraries(rkaudio_play librkaudio.so)
    target_link_libraries(rkaudio_play pthread)
endif()
