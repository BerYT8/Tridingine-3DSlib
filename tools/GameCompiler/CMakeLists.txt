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

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
# BUILD
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

            include
            "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    # -------------------------------------------------
    # Tridingine
    # -------------------------------------------------

    target_link_libraries(
        ${GAME_TARGET}
        PRIVATE

            "${TRIDINGINE_3DS_LIB}"
            "${ENTRYPOINT_3DS_LIB}"
            citro2d
            citro3d
            ctru
            m

            opusfile
            opus
            ogg
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

    # =================================================
    # Windows
    # =================================================

    if(WIN32)

        message(STATUS "Platform: Windows")

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

        # -------------------------------------------------
        # Check files
        # -------------------------------------------------

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

                include
                "${CMAKE_CURRENT_SOURCE_DIR}"
        )

        # -------------------------------------------------
        # Tridingine
        # -------------------------------------------------

        target_link_libraries(
            ${GAME_NAME}
            PRIVATE

                "${TRIDINGINE_PC_LIB}"
                "${ENTRYPOINT_PC_LIB}"
        )

        # -------------------------------------------------
        # Copy DLL next to executable
        # -------------------------------------------------

        add_custom_command(
            TARGET ${GAME_NAME}
            POST_BUILD

            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${TRIDINGINE_PC_DLL}"
                "$<TARGET_FILE_DIR:${GAME_NAME}>"

            COMMENT
                "Copying Tridingine.dll next to ${GAME_NAME}"
        )

        # -------------------------------------------------
        # Game title
        # -------------------------------------------------

        target_compile_definitions(
            ${GAME_NAME}
            PRIVATE

                GAME_TITLE="${GAME_TITLE}"
        )

    # =================================================
    # Linux
    # =================================================

    elseif(UNIX AND NOT APPLE)

        message(STATUS "Platform: Linux")

        set(
            TRIDINGINE_PC_LIB
            "${CMAKE_SOURCE_DIR}/lib/pc/libTridingine.so"
        )

        set(
            ENTRYPOINT_PC_LIB
            "${CMAKE_SOURCE_DIR}/lib/pc/libEntrypoint.a"
        )

        # -------------------------------------------------
        # Check files
        # -------------------------------------------------

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

                include
                "${CMAKE_CURRENT_SOURCE_DIR}"
        )

        # -------------------------------------------------
        # Tridingine
        # -------------------------------------------------

        target_link_libraries(
            ${GAME_NAME}
            PRIVATE

                "${TRIDINGINE_PC_LIB}"
                "${ENTRYPOINT_PC_LIB}"
        )

        # -------------------------------------------------
        # Copy SO next to executable
        # -------------------------------------------------

        add_custom_command(
            TARGET ${GAME_NAME}
            POST_BUILD

            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${TRIDINGINE_PC_LIB}"
                "$<TARGET_FILE_DIR:${GAME_NAME}>"

            COMMENT
                "Copying libTridingine.so next to ${GAME_NAME}"
        )

        # -------------------------------------------------
        # Make executable search for .so beside itself
        # -------------------------------------------------

        set_target_properties(
            ${GAME_NAME}
            PROPERTIES

                BUILD_RPATH
                    "$ORIGIN"

                INSTALL_RPATH
                    "$ORIGIN"
        )

        # -------------------------------------------------
        # Game title
        # -------------------------------------------------

        target_compile_definitions(
            ${GAME_NAME}
            PRIVATE

                GAME_TITLE="${GAME_TITLE}"
        )

    else()

        message(FATAL_ERROR
            "Unsupported PC platform."
        )

    endif()

endif()