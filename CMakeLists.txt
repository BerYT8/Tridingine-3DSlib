cmake_minimum_required(VERSION 3.16)

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

    src/net/socket.cpp
    src/net/tls.cpp
    src/net/tcp.cpp
    src/net/http.cpp

    src/system_memory.cpp
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
# CONFIGURATION
# =====================================================

message(STATUS "========================================")
message(STATUS "Building Tridingine")
message(STATUS "========================================")
message(STATUS "  Platform : ${TRIDINGINE_PLATFORM}")
message(STATUS "  Version  : ${PROJECT_VERSION}")
message(STATUS "========================================")


# =====================================================
# PC
# =====================================================

if(NOT BUILD_3DS)

    message(STATUS "Configuring PC build")

    # -------------------------------------------------
    # OpenGL
    # -------------------------------------------------

    find_package(OpenGL REQUIRED)

    # -------------------------------------------------
    # OpenSSL
    # -------------------------------------------------

    find_package(OpenSSL REQUIRED)

    # -------------------------------------------------
    # GLEW
    # -------------------------------------------------
    #
    # Windows:
    #   Busca una instalación precompilada indicada mediante
    #   la variable de entorno CMAKE_PREFIX_PATH.
    #
    # Ejemplo PowerShell:
    #
    #   $env:CMAKE_PREFIX_PATH="C:/glew-2.3.1"
    #
    # Linux / macOS:
    #   Usa find_package(GLEW).
    #
    # -------------------------------------------------

    if(WIN32)

        message(STATUS "Searching for precompiled GLEW...")

        # -------------------------------------------------
        # Obtener CMAKE_PREFIX_PATH del entorno
        # -------------------------------------------------

        if(DEFINED ENV{CMAKE_PREFIX_PATH})

            set(GLEW_PREFIX_PATH
                "$ENV{CMAKE_PREFIX_PATH}"
            )

        else()

            set(GLEW_PREFIX_PATH "")

        endif()

        message(
            STATUS
            "CMAKE_PREFIX_PATH: ${GLEW_PREFIX_PATH}"
        )

        # -------------------------------------------------
        # Detectar arquitectura
        # -------------------------------------------------

        if(CMAKE_SIZEOF_VOID_P EQUAL 8)

            set(GLEW_ARCH "x64")

        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)

            set(GLEW_ARCH "Win32")

        else()

            message(FATAL_ERROR
                "Unable to determine target architecture."
            )

        endif()

        message(
            STATUS
            "GLEW architecture: ${GLEW_ARCH}"
        )

        # -------------------------------------------------
        # Buscar GLEW
        # -------------------------------------------------

        set(GLEW_ROOT "")

        # CMAKE_PREFIX_PATH puede contener varios paths
        # separados por ';'
        foreach(PREFIX ${GLEW_PREFIX_PATH})

            if(EXISTS
                "${PREFIX}/include/GL/glew.h"
            )

                set(GLEW_ROOT
                    "${PREFIX}"
                )

                break()

            endif()

        endforeach()

        # -------------------------------------------------
        # Comprobar headers
        # -------------------------------------------------

        if(NOT GLEW_ROOT)

            message(FATAL_ERROR
                "GLEW headers not found.\n\n"
                "CMAKE_PREFIX_PATH:\n"
                "  ${GLEW_PREFIX_PATH}\n\n"
                "Expected:\n"
                "  <prefix>/include/GL/glew.h\n\n"
                "Example:\n"
                "  CMAKE_PREFIX_PATH=C:/glew-2.3.1"
            )

        endif()

        message(
            STATUS
            "GLEW root: ${GLEW_ROOT}"
        )

        # -------------------------------------------------
        # Buscar glew32s.lib
        # -------------------------------------------------

        find_library(
            GLEW_STATIC_LIBRARY
            NAMES
                glew32s
            PATHS
                "${GLEW_ROOT}/lib/Release/${GLEW_ARCH}"
            NO_DEFAULT_PATH
            REQUIRED
        )

        message(
            STATUS
            "GLEW static library: ${GLEW_STATIC_LIBRARY}"
        )

        # -------------------------------------------------
        # Crear target compatible con el resto del proyecto
        # -------------------------------------------------

        if(NOT TARGET GLEW::Static)

            add_library(
                GLEW::Static
                STATIC
                IMPORTED GLOBAL
            )

            set_target_properties(
                GLEW::Static
                PROPERTIES

                    IMPORTED_LOCATION
                        "${GLEW_STATIC_LIBRARY}"

                    INTERFACE_INCLUDE_DIRECTORIES
                        "${GLEW_ROOT}/include"

                    INTERFACE_COMPILE_DEFINITIONS
                        GLEW_STATIC
            )

        endif()

    else()

        # -------------------------------------------------
        # Linux / macOS
        # -------------------------------------------------

        message(STATUS "Searching for system GLEW...")

        find_package(
            GLEW
            REQUIRED
        )

        # Normalizar el nombre del target
        if(TARGET GLEW::Static)

            # Ya existe

        elseif(TARGET GLEW::GLEW)

            add_library(
                GLEW::Static
                ALIAS
                GLEW::GLEW
            )

        elseif(TARGET GLEW::glew)

            add_library(
                GLEW::Static
                ALIAS
                GLEW::glew
            )

        else()

            message(FATAL_ERROR
                "GLEW was found, but no compatible GLEW target exists."
            )

        endif()

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

    set(
        SDL2IMAGE_AVIF 
        OFF 
        CACHE BOOL 
        "" 
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
            WINDOWS_EXPORT_ALL_SYMBOLS ON
    )

    target_compile_definitions(
        TridingineCore
        PRIVATE
            PLATFORM_PC
    )

    target_include_directories(
        TridingineCore
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/lib_include
    )

    target_link_libraries(
        TridingineCore
        PRIVATE

            # GLEW compilado desde external/glew
            GLEW::Static

            # OpenGL
            OpenGL::GLU
            OpenGL::GL

            # SDL
            SDL2::SDL2-static
            SDL2_mixer::SDL2_mixer-static
            SDL2_image::SDL2_image-static
            SDL2_ttf::SDL2_ttf-static

            OpenSSL::SSL
            OpenSSL::Crypto
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
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/lib_include
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
# NINTENDO 3DS
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
            OUTPUT_NAME "${TRIDINGINE_OUTPUT_NAME}"
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
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/lib_include
            $ENV{DEVKITPRO}/portlibs/3ds/include
    )

    target_link_directories(
        TridingineCore
        PUBLIC
            $ENV{DEVKITPRO}/portlibs/3ds/lib
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
    # y no debe aparecer en un paquete instalado.
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
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/lib_include
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

            mbedtls
            mbedx509
            mbedcrypto

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

    message(STATUS "  GLEW     : external/glew")
    message(STATUS "  GLEW     : static")

endif()

message(STATUS "========================================")