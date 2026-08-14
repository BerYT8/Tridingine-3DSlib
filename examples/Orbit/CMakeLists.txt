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
    Player.cpp
    Collision.cpp
    Enemy.cpp
    Game.cpp
    Pickup.cpp
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
        QUIET
    )

    if(Tridingine_FOUND)

        message(STATUS "Tridingine 3DS found through CMake.")

    else()

        message(STATUS
            "Tridingine 3DS not found through CMake."
        )

        set(
            TRIDINGINE_3DS_LIB
            "${CMAKE_SOURCE_DIR}/lib/3ds/tridingine.a"
        )

        set(
            ENTRYPOINT_3DS_LIB
            "${CMAKE_SOURCE_DIR}/lib/3ds/entrypoint.a"
        )

        if(NOT EXISTS "${TRIDINGINE_3DS_LIB}")

            message(FATAL_ERROR
                "Tridingine 3DS library not found:\n"
                "  ${TRIDINGINE_3DS_LIB}"
            )

        endif()

        if(NOT EXISTS "${ENTRYPOINT_3DS_LIB}")

            message(FATAL_ERROR
                "Tridingine 3DS entrypoint library not found:\n"
                "  ${ENTRYPOINT_3DS_LIB}"
            )

        endif()

        add_library(
            TridingineFallback3DS
            STATIC
            IMPORTED
        )

        set_target_properties(
            TridingineFallback3DS
            PROPERTIES
                IMPORTED_LOCATION
                    "${TRIDINGINE_3DS_LIB}"
        )

        add_library(
            EntryPointFallback3DS
            STATIC
            IMPORTED
        )

        set_target_properties(
            EntryPointFallback3DS
            PROPERTIES
                IMPORTED_LOCATION
                    "${ENTRYPOINT_3DS_LIB}"
        )

        add_library(
            Tridingine::Tridingine
            INTERFACE
            IMPORTED
        )

        set_target_properties(
            Tridingine::Tridingine
            PROPERTIES
                INTERFACE_LINK_LIBRARIES
                    "TridingineFallback3DS;EntryPointFallback3DS"
        )

    endif()

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
        QUIET
    )

    if(Tridingine_FOUND)

        message(STATUS "Tridingine PC found through CMake.")

    else()

        message(STATUS
            "Tridingine PC not found through CMake."
        )

        # =============================================
        # Windows
        # =============================================

        if(WIN32)

            set(
                TRIDINGINE_PC_LIB
                "${CMAKE_SOURCE_DIR}/lib/pc/libTridingine.lib"
            )

            set(
                ENTRYPOINT_PC_LIB
                "${CMAKE_SOURCE_DIR}/lib/pc/libEntrypoint.lib"
            )

            set(
                TRIDINGINE_PC_DLL
                "${CMAKE_SOURCE_DIR}/lib/pc/Tridingine.dll"
            )

            if(NOT EXISTS "${TRIDINGINE_PC_LIB}")

                message(FATAL_ERROR
                    "Tridingine Windows library not found:\n"
                    "  ${TRIDINGINE_PC_LIB}"
                )

            endif()

            if(NOT EXISTS "${ENTRYPOINT_PC_LIB}")

                message(FATAL_ERROR
                    "Tridingine Windows entrypoint library not found:\n"
                    "  ${ENTRYPOINT_PC_LIB}"
                )

            endif()

            if(NOT EXISTS "${TRIDINGINE_PC_DLL}")

                message(FATAL_ERROR
                    "Tridingine Windows DLL not found:\n"
                    "  ${TRIDINGINE_PC_DLL}"
                )

            endif()

            add_library(
                TridingineFallbackPC
                STATIC
                IMPORTED
            )

            set_target_properties(
                TridingineFallbackPC
                PROPERTIES
                    IMPORTED_LOCATION
                        "${TRIDINGINE_PC_LIB}"
            )

            add_library(
                EntryPointFallbackPC
                STATIC
                IMPORTED
            )

            set_target_properties(
                EntryPointFallbackPC
                PROPERTIES
                    IMPORTED_LOCATION
                        "${ENTRYPOINT_PC_LIB}"
            )

            add_library(
                Tridingine::Tridingine
                INTERFACE
                IMPORTED
            )

            set_target_properties(
                Tridingine::Tridingine
                PROPERTIES
                    INTERFACE_LINK_LIBRARIES
                        "TridingineFallbackPC;EntryPointFallbackPC"
            )

        # =============================================
        # Linux
        # =============================================

        elseif(UNIX)

            set(
                TRIDINGINE_PC_LIB
                "${CMAKE_SOURCE_DIR}/lib/pc/libTridingine.so"
            )

            set(
                ENTRYPOINT_PC_LIB
                "${CMAKE_SOURCE_DIR}/lib/pc/libEntrypoint.a"
            )

            if(NOT EXISTS "${TRIDINGINE_PC_LIB}")

                message(FATAL_ERROR
                    "Tridingine Linux library not found:\n"
                    "  ${TRIDINGINE_PC_LIB}"
                )

            endif()

            if(NOT EXISTS "${ENTRYPOINT_PC_LIB}")

                message(FATAL_ERROR
                    "Tridingine Linux entrypoint library not found:\n"
                    "  ${ENTRYPOINT_PC_LIB}"
                )

            endif()

            add_library(
                TridingineFallbackPC
                SHARED
                IMPORTED
            )

            set_target_properties(
                TridingineFallbackPC
                PROPERTIES
                    IMPORTED_LOCATION
                        "${TRIDINGINE_PC_LIB}"
            )

            add_library(
                EntryPointFallbackPC
                STATIC
                IMPORTED
            )

            set_target_properties(
                EntryPointFallbackPC
                PROPERTIES
                    IMPORTED_LOCATION
                        "${ENTRYPOINT_PC_LIB}"
            )

            add_library(
                Tridingine::Tridingine
                INTERFACE
                IMPORTED
            )

            set_target_properties(
                Tridingine::Tridingine
                PROPERTIES
                    INTERFACE_LINK_LIBRARIES
                        "TridingineFallbackPC;EntryPointFallbackPC"
            )

        else()

            message(FATAL_ERROR
                "Unsupported PC platform."
            )

        endif()

    endif()

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
    # Windows DLL
    # -------------------------------------------------

    if(WIN32 AND NOT Tridingine_FOUND)

        add_custom_command(
            TARGET ${GAME_NAME}
            POST_BUILD

            COMMAND
                ${CMAKE_COMMAND}
                -E copy_if_different

                "${CMAKE_SOURCE_DIR}/lib/pc/Tridingine.dll"

                "$<TARGET_FILE_DIR:${GAME_NAME}>/Tridingine.dll"

            COMMENT
                "Copying Tridingine.dll next to executable"
        )

    endif()

    # -------------------------------------------------
    # Game title
    # -------------------------------------------------

    target_compile_definitions(
        ${GAME_NAME}
        PRIVATE

            GAME_TITLE="${GAME_TITLE}"
    )

endif()