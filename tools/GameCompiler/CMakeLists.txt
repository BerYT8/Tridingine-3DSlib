cmake_minimum_required(VERSION 3.10)

project(
    Game
    LANGUAGES C CXX
)

# =====================================================
# OPTIONS
# =====================================================

option(
    BUILD_3DS
    "Build game for Nintendo 3DS"
    OFF
)

# =====================================================
# C++
# =====================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =====================================================
# GAME NAME
# =====================================================

set(
    GAME_NAME
    "${PROJECT_NAME}"
    CACHE STRING
    "Name of the game executable"
)

# =====================================================
# GAME TITLE
# =====================================================

set(
    GAME_TITLE
    "Game"
    CACHE STRING
    "Game title"
)

# =====================================================
# SOURCES
# =====================================================

set(SOURCES
)

# =====================================================
# TRIDINGINE
# =====================================================

if(BUILD_3DS)

    message(STATUS "========================================")
    message(STATUS "Building game for Nintendo 3DS")
    message(STATUS "========================================")

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

    # -------------------------------------------------
    # Tridingine 3DS
    # -------------------------------------------------

    find_package(
        Tridingine
        CONFIG
        REQUIRED
    )

    # -------------------------------------------------
    # Executable
    # -------------------------------------------------

    set(
        GAME_TARGET
        code
    )

    add_executable(
        ${GAME_TARGET}

        ${SOURCES}
        main.cpp
    )

    target_include_directories(
        ${GAME_TARGET}
        PRIVATE

            "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    # -------------------------------------------------
    # Tridingine
    # -------------------------------------------------

    target_link_libraries(
        ${GAME_TARGET}
        PRIVATE

            Tridingine::Tridingine
    )

    # -------------------------------------------------
    # Game title
    # -------------------------------------------------

    target_compile_definitions(
        ${GAME_TARGET}
        PRIVATE

            GAME_TITLE="${GAME_TITLE}"
    )

else()

    message(STATUS "========================================")
    message(STATUS "Building game for PC")
    message(STATUS "========================================")

    # -------------------------------------------------
    # Tridingine PC
    # -------------------------------------------------

    find_package(
        Tridingine
        CONFIG
        REQUIRED
    )

    # -------------------------------------------------
    # Executable
    # -------------------------------------------------

    add_executable(
        ${GAME_NAME}

        ${SOURCES}
        main.cpp
    )

    target_include_directories(
        ${GAME_NAME}
        PRIVATE

            "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    # -------------------------------------------------
    # Tridingine
    # -------------------------------------------------

    target_link_libraries(
        ${GAME_NAME}
        PRIVATE

            Tridingine::Tridingine
    )

    # -------------------------------------------------
    # Game title
    # -------------------------------------------------

    target_compile_definitions(
        ${GAME_NAME}
        PRIVATE

            GAME_TITLE="${GAME_TITLE}"
    )

endif()