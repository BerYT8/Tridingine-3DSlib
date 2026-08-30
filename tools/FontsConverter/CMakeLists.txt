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
# ImageMagick
# --------------------------------------------------------------------------

find_package(ImageMagick REQUIRED COMPONENTS Magick++)

# --------------------------------------------------------------------------
# Executable
# --------------------------------------------------------------------------

add_executable(${PROJECT_NAME}
    main.cpp
    source/bcfnt.cpp
    source/freetype.cpp
    source/magick_compat.cpp
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
        ImageMagick::Magick++
)