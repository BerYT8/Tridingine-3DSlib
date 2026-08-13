cmake_minimum_required(VERSION 3.10)

project(3ds_libs C CXX)

# ---------------- OPCIÓN ----------------
option(BUILD_3DS "Build for Nintendo 3DS" OFF)

# ---------------- C++ STANDARD ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------- SOURCES ----------------
set(SOURCES
    src/romfs_path.h
    src/romfs_path.cpp

    src/console/drawConsole.h
    src/console/drawConsole.cpp
    src/console/console.cpp

    src/system_language.cpp
    src/system_time.cpp
    src/sys_fonts_gen_funcs.h
    src/system_fonts.cpp

    src/entrypoint.cpp
    src/pak_loader/pak_loader.cpp

    src/inputIntern.h
    src/input.cpp
    src/localization/localization_sys.h
    src/localization/localization.cpp
    src/delta_time.c
    src/config.h
    src/config.c

    src/screens/screensLoadingStart.h
    src/screens/screensLoadingStart.cpp
    src/screens/screensValues.h
    src/screens/modfiy_screens.cpp
    src/screens/screens.cpp

    src/draw/shaders/opengl/opengl.shader.h
    src/draw/shaders/opengl/opengl.shader.cpp
    src/draw/shaders/pica200/pica.shader.h
    src/draw/shaders/pica200/pica.shader.cpp
    src/draw/2d/2d_shapes.c
    src/draw/2d/2d_vals.h
    src/draw/2d/2d_text.cpp
    src/draw/3d/3d_shapes.cpp

    src/html/html.h
    src/html/html.cpp
    src/html/html-ds.cpp

    src/save_system/types.h
    src/save_system/types.c
    src/save_system/saveSys.c
    src/save_system/loadSys.c
    src/save_system/asyncFuncs.cpp
    src/random/randomIntern.h
    src/random/randomIntern.cpp
    src/random/random.cpp
    src/nlohmann/json.hpp
    src/textures/json.h
    src/textures/json.cpp
    src/textures/sprites.h
    src/textures/sprites.cpp
    src/textures/textures.c

    src/sound/ogg/config_types.h
    src/sound/ogg/os_types.h
    src/sound/ogg/ogg.h

    src/sound/opus_defines.h
    src/sound/opus_multistream.h
    src/sound/opus.h
    src/sound/opusfile.h
    src/sound/opus_types.h
    src/sound/opus_projection.h

    src/sound/soundLoad.h
    src/sound/sound_wrapper.cpp
    src/sound/sound.c
    src/animation/animation.c
)

file(GLOB_RECURSE IMGUI_SRC "lib_include/imgui/*.cpp")

# =====================================================
# 🖥️ PC BUILD CONFIGURATION
# =====================================================
if(NOT BUILD_3DS)

    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)

    # -------------------------------------------------
    # SDL2 - STATIC
    # -------------------------------------------------
    set(SDL_SHARED OFF CACHE BOOL "Build SDL2 shared library" FORCE)
    set(SDL_STATIC ON CACHE BOOL "Build SDL2 static library" FORCE)
    set(SDL_STATIC_PIC ON CACHE BOOL "" FORCE)

    add_subdirectory(
        external/sdl2
        EXCLUDE_FROM_ALL
    )

    # -------------------------------------------------
    # SDL2_mixer - STATIC
    # -------------------------------------------------
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

    add_subdirectory(
        external/sdl2-mixer
        EXCLUDE_FROM_ALL
    )

    # -------------------------------------------------
    # SDL2_image - STATIC
    # -------------------------------------------------
    set(SDL2IMAGE_BUILD_SHARED_LIBS OFF CACHE BOOL
        "Build SDL2_image shared library" FORCE)

    set(SDL2IMAGE_BUILD_STATIC_LIBS ON CACHE BOOL
        "Build SDL2_image static library" FORCE)

    add_subdirectory(
        external/sdl2-image
        EXCLUDE_FROM_ALL
    )

    # -------------------------------------------------
    # SDL2_ttf - STATIC
    # -------------------------------------------------
    set(SDL2TTF_BUILD_SHARED_LIBS OFF CACHE BOOL
        "Build SDL2_ttf shared library" FORCE)

    set(SDL2TTF_BUILD_STATIC_LIBS ON CACHE BOOL
        "Build SDL2_ttf static library" FORCE)

    add_subdirectory(
        external/sdl2-ttf
        EXCLUDE_FROM_ALL
    )

    # -------------------------------------------------
    # Main library
    # -------------------------------------------------
    add_library(${PROJECT_NAME} SHARED
        ${SOURCES}
    )

    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            PLATFORM_PC
    )

    target_include_directories(${PROJECT_NAME}
        PRIVATE
            include
            ${OPENGL_INCLUDE_DIR}
    )

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            GLEW::GLEW
            OpenGL::GLU
            OpenGL::GL

            SDL2::SDL2-static
            SDL2_mixer::SDL2_mixer-static
            SDL2_image::SDL2_image-static
            SDL2_ttf::SDL2_ttf-static
    )


# =====================================================
# 🎮 3DS BUILD (OFICIAL)
# =====================================================
else()
    set(LIBS_3DS
        ${CMAKE_SOURCE_DIR}/lib/3ds/libopusfile.a
        ${CMAKE_SOURCE_DIR}/lib/3ds/libopus.a
        ${CMAKE_SOURCE_DIR}/lib/3ds/libogg.a
    )

    message(STATUS "Building for 3DS with devkitPro")

    include("$ENV{DEVKITPRO}/cmake/3DS.cmake")

    add_library(${PROJECT_NAME}
        ${SOURCES}
    )

    target_include_directories(${PROJECT_NAME} PRIVATE
        include
        lib_include
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE
        __3DS__
        PLATFORM_3DS
    )

    ctr_add_shader_library(3dshader
        src/draw/3d/3dshader.v.pica
    )

    dkp_add_embedded_binary_library(3dlibrary
        3dshader
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        3dlibrary
        citro2d
        citro3d
        ctru
        m
        ${LIBS_3DS}
    )
endif()
