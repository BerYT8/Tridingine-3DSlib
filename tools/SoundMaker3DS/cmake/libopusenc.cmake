cmake_minimum_required(VERSION 3.10)

project(opusenc
    VERSION 0.2.1
    LANGUAGES C
)

# ------------------------------------------------------------
# Opus
# ------------------------------------------------------------

if(WIN32)
    find_library(OPUS_LIBRARY
        NAMES opus opus_static libopus
        PATHS
            "${CMAKE_CURRENT_SOURCE_DIR}/../lib"
            "${CMAKE_CURRENT_SOURCE_DIR}/../libopus/build"
    )
else()
    find_library(OPUS_LIBRARY
        NAMES opus
        PATHS
            "${CMAKE_CURRENT_SOURCE_DIR}/../lib"
            "${CMAKE_CURRENT_SOURCE_DIR}/../libopus/build"
    )
endif()

if(NOT OPUS_LIBRARY)
    message(FATAL_ERROR
        "No se encontró libopus/opus.lib. "
        "OPUS_LIBRARY=${OPUS_LIBRARY}"
    )
endif()

message(STATUS "Using Opus library: ${OPUS_LIBRARY}")


# ------------------------------------------------------------
# opusenc
# ------------------------------------------------------------

add_library(opusenc
    src/ogg_packer.c
    src/opus_header.c
    src/opusenc.c
    src/picture.c
    src/resample.c
    src/unicode_support.c
)

target_compile_definitions(opusenc PRIVATE
    OUTSIDE_SPEEX
    RANDOM_PREFIX=opusenc
    PACKAGE_NAME="${PROJECT_NAME}"
    PACKAGE_VERSION="${PROJECT_VERSION}"
)

target_include_directories(opusenc PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_include_directories(opusenc PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/lib_include"
)

target_link_libraries(opusenc PRIVATE
    "${OPUS_LIBRARY}"
)