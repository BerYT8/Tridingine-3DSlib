cmake_minimum_required(VERSION 3.10)

project(opusenc
    VERSION 0.2.1
    LANGUAGES C
)

# ------------------------------------------------------------
# opusenc
# ------------------------------------------------------------

add_library(opusenc
    src/ogg_packer.c
    src/opus_header.c
    src/opusenc.c
    src/picture.c
    src/random.c
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
    opus
)