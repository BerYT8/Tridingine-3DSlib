cmake_minimum_required(VERSION 3.10)

project(3ds_libs C CXX)

# ---------------- OPCIÓN ----------------
option(BUILD_3DS "Build for Nintendo 3DS" OFF)

# ---------------- C++ ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------- SOURCES ----------------
set(SOURCES
    src/system_language.cpp
    src/system_time.cpp

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
    src/screens/screens.c

    src/draw/shaders/opengl/opengl.shader.h
    src/draw/shaders/opengl/opengl.shader.cpp
    src/draw/shaders/pica200/pica.shader.h
    src/draw/shaders/pica200/pica.shader.cpp
    src/draw/2d/2d_shapes.c
    src/draw/2d/2d_vals.h
    src/draw/2d/2d_text.cpp
    #src/draw/3d/3dshader_shbin.h
    src/draw/3d/3d_shapes.cpp

    src/ui/definitions.h
    src/ui/canvas/canvas.c
    src/ui/user_interface.c

    src/save_system/types.h
    src/save_system/types.c
    #src/save_system/loadSysL.h
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
    #src/console/drawConsole.h
    #src/console/drawConsole.cpp
    #src/console/console.cpp
)

file(GLOB_RECURSE IMGUI_SRC "lib_include/imgui/*.cpp")

# =====================================================
# 🖥️ PC BUILD
# =====================================================
if(NOT BUILD_3DS)
    file(GLOB_RECURSE GLEW_LIB_DLL "lib/glew/x64/glew32.lib")
    file(GLOB_RECURSE GLEW_LIB_STATIC "lib/glew/x64/glew32s.lib")

    add_library(${PROJECT_NAME} 
        ${SOURCES}
        #${IMGUI_SRC}
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_PC)

    target_include_directories(${PROJECT_NAME} PRIVATE 
        include
        lib_include
    )

    file(GLOB_RECURSE libs "lib/x64/*.lib")

    target_link_libraries(${PROJECT_NAME} PRIVATE 
        ${libs}
        opengl32.lib
        ${GLEW_LIB_STATIC}
        glu32.lib
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
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