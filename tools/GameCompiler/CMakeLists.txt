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
    add_executable(${PROJECT_NAME} WIN32
        ${SOURCES}
        main.cpp
    )

    target_include_directories(${PROJECT_NAME} PRIVATE 
        include
    )

    file(GLOB_RECURSE libs "lib/pc/*.lib")

    target_link_libraries(${PROJECT_NAME} PRIVATE 
        ${libs}
        opengl32.lib
        glu32.lib
    )

else()
    message(STATUS "Building for 3DS with devkitPro")

    include("$ENV{DEVKITPRO}/cmake/3DS.cmake")

    set(LIBS_3DS
        ${CMAKE_SOURCE_DIR}/lib/3ds/3ds_libs.a
    )

    set(NAME "code")

    add_executable(${NAME}
        ${SOURCES}
        main.cpp
    )

    target_include_directories(${NAME} PRIVATE
        include
    )

    target_link_libraries(${NAME}
        ${LIBS_3DS}
        citro2d
        citro3d
        ctru
        m
    )


endif()