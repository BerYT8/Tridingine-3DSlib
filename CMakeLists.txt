cmake_minimum_required(VERSION 3.10)

project(
    Tridingine
    VERSION 1.0.0
    LANGUAGES C CXX
)

# =====================================================
# OPCIONES
# =====================================================

option(
    BUILD_3DS
    "Build Tridingine for Nintendo 3DS"
    OFF
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =====================================================
# C++
# =====================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =====================================================
# OUTPUT
# =====================================================

set(TRIDINGINE_OUTPUT_NAME "Tridingine")

# =====================================================
# SOURCES
# =====================================================

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

# =====================================================
# PLATFORM
# =====================================================

if(BUILD_3DS)

    set(TRIDINGINE_PLATFORM "3DS")

else()

    if(WIN32)

        set(TRIDINGINE_PLATFORM "Win")

    elseif(APPLE)

        set(TRIDINGINE_PLATFORM "Mac")

    elseif(UNIX)

        set(TRIDINGINE_PLATFORM "Linux")

    else()

        message(FATAL_ERROR
            "Unsupported PC platform"
        )

    endif()

endif()

# =====================================================
# CONFIGURATION HEADER
# =====================================================

message(STATUS "========================================")
message(STATUS "Building Tridingine")
message(STATUS "========================================")
message(STATUS "  Platform : ${TRIDINGINE_PLATFORM}")
message(STATUS "  Version  : ${PROJECT_VERSION}")
message(STATUS "========================================")


# =====================================================
# 🖥️ PC
# =====================================================

if(NOT BUILD_3DS)

    message(STATUS "Configuring PC build")

    # -------------------------------------------------
    # OpenGL
    # -------------------------------------------------

    find_package(OpenGL REQUIRED)

    # =================================================
    # GLEW STATIC
    # =================================================
    #
    # GLEW se obtiene EXCLUSIVAMENTE desde:
    #
    #     external/glew/include
    #     external/glew/lib
    #
    # No se utiliza find_package(GLEW)
    # No se utiliza find_path()
    # No se utiliza find_library()
    #
    # Linux:
    #     lib/libGLEW.a
    #
    # Windows:
    #     lib/glew32s.lib
    #
    # =================================================

    set(GLEW_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/external/glew"
    )

    set(GLEW_INCLUDE_DIR
        "${GLEW_ROOT}/include"
    )

    if(WIN32)

        set(GLEW_STATIC_LIBRARY
            "${GLEW_ROOT}/lib/glew32s.lib"
        )

    elseif(UNIX AND NOT APPLE)

        set(GLEW_STATIC_LIBRARY
            "${GLEW_ROOT}/lib/libGLEW.a"
        )

    elseif(APPLE)

        set(GLEW_STATIC_LIBRARY
            "${GLEW_ROOT}/lib/libGLEW.a"
        )

    else()

        message(FATAL_ERROR
            "Unsupported platform for bundled GLEW."
        )

    endif()

    # -------------------------------------------------
    # Verificar GLEW
    # -------------------------------------------------

    if(NOT EXISTS "${GLEW_INCLUDE_DIR}/GL/glew.h")

        message(FATAL_ERROR
            "GLEW headers not found:\n"
            "  ${GLEW_INCLUDE_DIR}/GL/glew.h"
        )

    endif()

    if(NOT EXISTS "${GLEW_STATIC_LIBRARY}")

        message(FATAL_ERROR
            "Static GLEW library not found:\n"
            "  ${GLEW_STATIC_LIBRARY}"
        )

    endif()

    message(STATUS "GLEW root    : ${GLEW_ROOT}")
    message(STATUS "GLEW include : ${GLEW_INCLUDE_DIR}")
    message(STATUS "GLEW static  : ${GLEW_STATIC_LIBRARY}")

    # -------------------------------------------------
    # GLEW::GLEW
    # -------------------------------------------------

    add_library(
        GLEW::GLEW
        STATIC
        IMPORTED
    )

    set_target_properties(
        GLEW::GLEW
        PROPERTIES

            IMPORTED_LOCATION
                "${GLEW_STATIC_LIBRARY}"

            INTERFACE_INCLUDE_DIRECTORIES
                "${GLEW_INCLUDE_DIR}"
    )

    # -------------------------------------------------
    # GLEW Windows
    # -------------------------------------------------

    if(WIN32)

        target_compile_definitions(
            GLEW::GLEW
            INTERFACE
                GLEW_STATIC
        )

    endif()

    # -------------------------------------------------
    # SDL2
    # -------------------------------------------------

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
        external/sdl2
        EXCLUDE_FROM_ALL
    )

    # -------------------------------------------------
    # SDL2_mixer
    # -------------------------------------------------

    set(
        BUILD_SHARED_LIBS
        OFF
        CACHE BOOL
        "Build shared libraries"
        FORCE
    )

    add_subdirectory(
        external/sdl2-mixer
        EXCLUDE_FROM_ALL
    )

    # -------------------------------------------------
    # SDL2_image
    # -------------------------------------------------

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
        external/sdl2-image
        EXCLUDE_FROM_ALL
    )

    # -------------------------------------------------
    # SDL2_ttf
    # -------------------------------------------------

    set(
        SDL2TTF_BUILD_SHARED_LIBS
        OFF
        CACHE BOOL
        "Build SDL2_ttf shared library"
        FORCE
    )

    set(
        SDL2TTF_BUILD_STATIC_LIBS
        ON
        CACHE BOOL
        "Build SDL2_ttf static library"
        FORCE
    )

    add_subdirectory(
        external/sdl2-ttf
        EXCLUDE_FROM_ALL
    )

    # =================================================
    # CORE
    # =================================================

    add_library(
        TridingineCore
        SHARED
        ${SOURCES}
    )

    set_target_properties(
        TridingineCore
        PROPERTIES
            OUTPUT_NAME
                ${TRIDINGINE_OUTPUT_NAME}
    )

    target_compile_definitions(
        TridingineCore
        PRIVATE
            PLATFORM_PC
    )

    target_include_directories(
        TridingineCore
        PUBLIC

            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib_include>

            $<INSTALL_INTERFACE:include>

        PRIVATE

            ${OPENGL_INCLUDE_DIR}
    )

    target_link_libraries(
        TridingineCore
        PRIVATE

            GLEW::GLEW

            OpenGL::GLU
            OpenGL::GL

            SDL2::SDL2-static
            SDL2_mixer::SDL2_mixer-static
            SDL2_image::SDL2_image-static
            SDL2_ttf::SDL2_ttf-static
    )

    # =================================================
    # ENTRYPOINT
    # =================================================

    add_library(
        TridingineEntrypoint
        STATIC
        src/entrypoint.cpp
    )

    target_compile_definitions(
        TridingineEntrypoint
        PRIVATE
            PLATFORM_PC
    )

    target_include_directories(
        TridingineEntrypoint
        PUBLIC

            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib_include>

            $<INSTALL_INTERFACE:include>
    )

    # =================================================
    # PUBLIC TARGET
    # =================================================

    add_library(
        Tridingine
        INTERFACE
    )

    target_link_libraries(
        Tridingine
        INTERFACE

            TridingineCore
            TridingineEntrypoint
    )

    set_target_properties(
        Tridingine
        PROPERTIES
            EXPORT_NAME Tridingine
    )


# =====================================================
# 🎮 NINTENDO 3DS
# =====================================================

else()

    message(STATUS "Configuring Nintendo 3DS build")

    # -------------------------------------------------
    # devkitPro
    # -------------------------------------------------

    if(NOT DEFINED ENV{DEVKITPRO})

        message(FATAL_ERROR
            "DEVKITPRO environment variable is not set."
        )

    endif()

    include(
        "$ENV{DEVKITPRO}/cmake/3DS.cmake"
    )

    # =================================================
    # CORE
    # =================================================

    add_library(
        TridingineCore
        STATIC
        ${SOURCES}
    )

    set_target_properties(
        TridingineCore
        PROPERTIES
            OUTPUT_NAME
                ${TRIDINGINE_OUTPUT_NAME}
    )

    target_compile_definitions(
        TridingineCore
        PRIVATE
            __3DS__
            PLATFORM_3DS
    )

    target_include_directories(
        TridingineCore
        PUBLIC

            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib_include>

            $<INSTALL_INTERFACE:include>
    )

    # =================================================
    # SHADER
    # =================================================

    ctr_add_shader_library(
        3dshader
        src/draw/3d/3dshader.v.pica
    )

    dkp_add_embedded_binary_library(
        3dlibrary
        3dshader
    )

    set_target_properties(
        3dlibrary
        PROPERTIES
            OUTPUT_NAME
                TridingineShaderData
    )

    # =================================================
    # FIX CMAKE EXPORT
    # =================================================
    #
    # dkp_add_embedded_binary_library() añade el
    # directorio .dkp-generated al INTERFACE.
    #
    # Ese directorio solamente existe durante el build
    # y NO puede aparecer en un paquete instalado.
    #
    # El archivo .a sí se instala y se exporta.
    #

    set_target_properties(
        3dlibrary
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ""
    )

    # =================================================
    # ENTRYPOINT
    # =================================================

    add_library(
        TridingineEntrypoint
        STATIC
        src/entrypoint.cpp
    )

    target_compile_definitions(
        TridingineEntrypoint
        PRIVATE
            __3DS__
            PLATFORM_3DS
    )

    target_include_directories(
        TridingineEntrypoint
        PUBLIC

            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib_include>

            $<INSTALL_INTERFACE:include>
    )

    # =================================================
    # 3DS DEPENDENCIES
    # =================================================

    target_link_libraries(
        TridingineCore
        PUBLIC

            3dlibrary

            citro2d
            citro3d
            ctru
            m

            opusfile
            opus
            ogg
    )

    # =================================================
    # PUBLIC TARGET
    # =================================================

    add_library(
        Tridingine
        INTERFACE
    )

    target_link_libraries(
        Tridingine
        INTERFACE

            TridingineCore
            TridingineEntrypoint
    )

    set_target_properties(
        Tridingine
        PROPERTIES
            EXPORT_NAME Tridingine
    )

endif()

# =====================================================
# SUMMARY
# =====================================================

message(STATUS "========================================")
message(STATUS "Tridingine configuration:")
message(STATUS "  Platform : ${TRIDINGINE_PLATFORM}")
message(STATUS "  Version  : ${PROJECT_VERSION}")

if(NOT BUILD_3DS)

    message(STATUS "  GLEW     : bundled static")

    if(WIN32)
        message(STATUS "  GLEW lib : ${GLEW_STATIC_LIBRARY}")
    else()
        message(STATUS "  GLEW lib : ${GLEW_STATIC_LIBRARY}")
    endif()

endif()

message(STATUS "========================================")