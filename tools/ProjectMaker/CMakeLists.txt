cmake_minimum_required(VERSION 3.10)

project(ProjectMaker C CXX)

# ---------------- C++ ----------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------- PAK EMBEBIDO ----------------

set(PAK_FILE
    "${CMAKE_SOURCE_DIR}/template.pak"
)

set(GENERATED_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

set(PAK_HEADER
    "${GENERATED_DIR}/template_pak.h"
)

set(PAK_GENERATOR
    "${CMAKE_SOURCE_DIR}/cmake/bin2header.py"
)

file(MAKE_DIRECTORY "${GENERATED_DIR}")

add_custom_command(
    OUTPUT
        "${PAK_HEADER}"

    COMMAND
        python3
        "${PAK_GENERATOR}"
        "${PAK_FILE}"
        "${PAK_HEADER}"

    DEPENDS
        "${PAK_FILE}"
        "${PAK_GENERATOR}"

    COMMENT
        "Generando template_pak.h desde template.pak"
)

add_custom_target(generate_pak_header
    DEPENDS
        "${PAK_HEADER}"
)

# ---------------- SOURCES ----------------

set(SOURCES
    "${CMAKE_SOURCE_DIR}/../../src/pak_loader/pak_loader.cpp"
)

add_executable(${PROJECT_NAME}
    main.cpp
    ${SOURCES}
    ${PAK_HEADER}
)

add_dependencies(
    ${PROJECT_NAME}
    generate_pak_header
)

# ---------------- DEFINITIONS ----------------

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        PLATFORM_PC
)

# ---------------- INCLUDES ----------------

target_include_directories(${PROJECT_NAME}
    PRIVATE
        "${CMAKE_SOURCE_DIR}/../../include"
        "${GENERATED_DIR}"
)