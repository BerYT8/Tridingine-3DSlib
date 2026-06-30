cmake_minimum_required(VERSION 3.10)

project(SoundMaker3DS C CXX)

# ---------------- C++ ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------- SOURCES ----------------
set(SOURCES
    main.cpp
)

file(GLOB_RECURSE SOURCES_SRC "src/*.cpp" "src/*.c")
add_executable(${PROJECT_NAME} 
    #${IMGUI_SRC}
    ${SOURCES}
    ${SOURCES_SRC}
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    lib_include
    include
)

file(GLOB_RECURSE libs "libs/*.lib")

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${libs}
    #opengl32
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)