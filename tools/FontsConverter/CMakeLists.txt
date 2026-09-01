cmake_minimum_required(VERSION 3.16)

project(
    FontsConverter
    LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------------------------------
# Paths
# --------------------------------------------------------------------------

set(TRIDINGINE_ROOT
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# --------------------------------------------------------------------------
# FreeType
# --------------------------------------------------------------------------

add_subdirectory(
    freetype
)

# --------------------------------------------------------------------------
# SDL2
# --------------------------------------------------------------------------

set(
    BUILD_SHARED_LIBS
    OFF
    CACHE BOOL
    "Build shared libraries"
    FORCE
)

set(
    SDL_SHARED
    OFF
    CACHE BOOL
    "Build SDL2 shared library"
    FORCE
)

set(
    SDL_STATIC
    ON
    CACHE BOOL
    "Build SDL2 static library"
    FORCE
)

set(
    SDL_STATIC_PIC
    ON
    CACHE BOOL
    ""
    FORCE
)

add_subdirectory(
    ${TRIDINGINE_ROOT}/external/sdl2
    ${CMAKE_CURRENT_BINARY_DIR}/external/sdl2
    EXCLUDE_FROM_ALL
)

# --------------------------------------------------------------------------
# SDL2_image
# --------------------------------------------------------------------------

set(
    SDL2IMAGE_BUILD_SHARED_LIBS
    OFF
    CACHE BOOL
    "Build SDL2_image shared libraries"
    FORCE
)

set(
    SDL2IMAGE_BUILD_STATIC_LIBS
    ON
    CACHE BOOL
    "Build SDL2_image static libraries"
    FORCE
)

set(
    SDL2IMAGE_AVIF
    OFF
    CACHE BOOL
    "Disable AVIF"
    FORCE
)

add_subdirectory(
    ${TRIDINGINE_ROOT}/external/sdl2-image
    ${CMAKE_CURRENT_BINARY_DIR}/external/sdl2-image
    EXCLUDE_FROM_ALL
)

# --------------------------------------------------------------------------
# Verify targets
# --------------------------------------------------------------------------

if(TARGET SDL2::SDL2-static)
    message(STATUS "SDL2 static target: OK")
else()
    message(FATAL_ERROR
        "SDL2::SDL2-static was not created."
    )
endif()

if(TARGET SDL2_image::SDL2_image-static)
    message(STATUS "SDL2_image static target: OK")
else()
    message(FATAL_ERROR
        "SDL2_image::SDL2_image-static was not created."
    )
endif()

# --------------------------------------------------------------------------
# Executable
# --------------------------------------------------------------------------

add_executable(${PROJECT_NAME}
    main.cpp
    source/bcfnt.cpp
    source/freetype.cpp
    source/swizzle.cpp
    source/threadPool.cpp
)

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
        SDL_MAIN_HANDLED
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        freetype
        SDL2::SDL2-static
        SDL2_image::SDL2_image-static
)