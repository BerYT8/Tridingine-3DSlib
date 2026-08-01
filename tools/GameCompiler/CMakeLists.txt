cmake_minimum_required(VERSION 3.10)

project(Game C CXX)

# ---------------- OPCIÓN ----------------
option(BUILD_3DS "Build for Nintendo 3DS" OFF)

# ---------------- C++ ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------- SOURCES ----------------
set(SOURCES
)

# =====================================================
# 🖥️ PC BUILD
# =====================================================
if(NOT BUILD_3DS)
    if(NOT DEFINED GAME_NAME)
        # Asigna un valor por defecto si no se introduce en la terminal
        set(GAME_NAME ${PROJECT_NAME} CACHE STRING "Nombre del proyecto")
    endif()

    # 1. Buscar todas las dependencias requeridas del sistema
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_mixer REQUIRED)

    # Nota: SDL2_image y SDL2_ttf a veces requieren buscarse mediante pkg-config o sus nombres exactos
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_ttf REQUIRED)

    add_executable(${GAME_NAME}
        ${SOURCES}
        main.cpp
    )

    target_include_directories(${GAME_NAME}
        PRIVATE
            include
            "${CMAKE_SOURCE_DIR}"
    )

    target_link_libraries(${GAME_NAME}
        PRIVATE
            "${CMAKE_SOURCE_DIR}/lib/pc/lib3ds_libs.so"
    )

    # 2. Gestionar el título del juego y pasarlo como definición a C++
    if(NOT DEFINED GAME_TITLE)
        set(GAME_TITLE "Game")
    endif()

    # Se usan las comillas escapadas \\\" para que C++ lo reciba como un string literal
    target_compile_definitions(${GAME_NAME}
        PRIVATE
            GAME_TITLE="${GAME_TITLE}"
    )

else()

    message(STATUS "Building for 3DS with devkitPro")

    include("$ENV{DEVKITPRO}/cmake/3DS.cmake")

    set(LIBS_3DS
        ${CMAKE_SOURCE_DIR}/lib/3ds/3ds_libs.a
    )

    set(NAME code)

    add_executable(${NAME}
        ${SOURCES}
        main.cpp
    )

    target_include_directories(${NAME}
        PRIVATE
            include
    )

    target_link_libraries(${NAME}
        PRIVATE
            ${LIBS_3DS}
            citro2d
            citro3d
            ctru
            m
    )

endif()