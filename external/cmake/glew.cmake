cmake_minimum_required(VERSION 3.16)

project(GLEW
    VERSION 2.2.0
    LANGUAGES C
)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------

option(GLEW_BUILD_SHARED "Build the shared GLEW library" ON)
option(GLEW_BUILD_STATIC "Build the static GLEW library" ON)
option(GLEW_BUILD_UTILS "Build glewinfo and visualinfo" OFF)
option(GLEW_NO_GLU "Disable GLU support" ON)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

set(GLEW_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glew.c
)

set(GLEW_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GL/eglew.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GL/glew.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GL/glxew.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GL/wglew.h
)

# ---------------------------------------------------------------------------
# OpenGL
# ---------------------------------------------------------------------------

find_package(OpenGL REQUIRED)

# ---------------------------------------------------------------------------
# Common configuration
# ---------------------------------------------------------------------------

set(GLEW_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---------------------------------------------------------------------------
# Static library
# ---------------------------------------------------------------------------

if(GLEW_BUILD_STATIC)

    add_library(GLEW_STATIC STATIC
        ${GLEW_SOURCE}
        ${GLEW_HEADERS}
    )

    add_library(GLEW::Static ALIAS GLEW_STATIC)

    target_include_directories(GLEW_STATIC
        PUBLIC
            ${GLEW_INCLUDE_DIR}
    )

    target_compile_definitions(GLEW_STATIC
        PRIVATE
            GLEW_STATIC
    )

    if(GLEW_NO_GLU)
        target_compile_definitions(GLEW_STATIC
            PRIVATE
                GLEW_NO_GLU
        )
    endif()

    target_link_libraries(GLEW_STATIC
        PUBLIC
            OpenGL::GL
    )

    set_target_properties(GLEW_STATIC PROPERTIES
        OUTPUT_NAME glew
        POSITION_INDEPENDENT_CODE ON
    )

endif()

# ---------------------------------------------------------------------------
# Shared library
# ---------------------------------------------------------------------------

if(GLEW_BUILD_SHARED)

    add_library(GLEW_SHARED SHARED
        ${GLEW_SOURCE}
        ${GLEW_HEADERS}
    )

    add_library(GLEW::Shared ALIAS GLEW_SHARED)

    target_include_directories(GLEW_SHARED
        PUBLIC
            ${GLEW_INCLUDE_DIR}
    )

    target_compile_definitions(GLEW_SHARED
        PRIVATE
            GLEW_BUILD
    )

    if(GLEW_NO_GLU)
        target_compile_definitions(GLEW_SHARED
            PRIVATE
                GLEW_NO_GLU
        )
    endif()

    target_link_libraries(GLEW_SHARED
        PUBLIC
            OpenGL::GL
    )

    set_target_properties(GLEW_SHARED PROPERTIES
        OUTPUT_NAME glew
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )

endif()

# ---------------------------------------------------------------------------
# Utility programs
# ---------------------------------------------------------------------------

if(GLEW_BUILD_UTILS)

    # -----------------------------------------------------------------------
    # glewinfo
    # -----------------------------------------------------------------------

    add_executable(glewinfo
        src/glewinfo.c
    )

    target_include_directories(glewinfo
        PRIVATE
            ${GLEW_INCLUDE_DIR}
    )

    if(GLEW_NO_GLU)
        target_compile_definitions(glewinfo
            PRIVATE
                GLEW_NO_GLU
        )
    endif()

    if(GLEW_BUILD_SHARED)

        target_link_libraries(glewinfo
            PRIVATE
                GLEW_SHARED
        )

    elseif(GLEW_BUILD_STATIC)

        target_compile_definitions(glewinfo
            PRIVATE
                GLEW_STATIC
        )

        target_link_libraries(glewinfo
            PRIVATE
                GLEW_STATIC
        )

    endif()

    # -----------------------------------------------------------------------
    # visualinfo
    # -----------------------------------------------------------------------

    add_executable(visualinfo
        src/visualinfo.c
    )

    target_include_directories(visualinfo
        PRIVATE
            ${GLEW_INCLUDE_DIR}
    )

    if(GLEW_NO_GLU)
        target_compile_definitions(visualinfo
            PRIVATE
                GLEW_NO_GLU
        )
    endif()

    if(GLEW_BUILD_SHARED)

        target_link_libraries(visualinfo
            PRIVATE
                GLEW_SHARED
        )

    elseif(GLEW_BUILD_STATIC)

        target_compile_definitions(visualinfo
            PRIVATE
                GLEW_STATIC
        )

        target_link_libraries(visualinfo
            PRIVATE
                GLEW_STATIC
        )

    endif()

endif()