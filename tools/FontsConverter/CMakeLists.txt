cmake_minimum_required(VERSION 3.12)

project(FontsConverter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------------------------------
# FreeType
# --------------------------------------------------------------------------

add_subdirectory(freetype)

# --------------------------------------------------------------------------
# SDL2
# --------------------------------------------------------------------------

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
    "Build SDL2 static library with PIC"
    FORCE
)

add_subdirectory(
    ../../external/sdl2
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
    "Build SDL2_image shared library"
    FORCE
)

set(
    SDL2IMAGE_BUILD_STATIC_LIBS
    ON
    CACHE BOOL
    "Build SDL2_image static library"
    FORCE
)

add_subdirectory(
    ../../external/sdl2-image
    ${CMAKE_CURRENT_BINARY_DIR}/external/sdl2-image
    EXCLUDE_FROM_ALL
)

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