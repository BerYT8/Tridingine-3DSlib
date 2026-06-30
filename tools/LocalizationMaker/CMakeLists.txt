cmake_minimum_required(VERSION 3.10)

project(LocalizationMaker C CXX)

# ---------------- C++ ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------- SOURCES ----------------
set(SOURCES
    json.hpp
)

add_executable(${PROJECT_NAME} 
    ${SOURCES}
    main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    lib_include
    include
)

file(GLOB_RECURSE libs "lib/x64/*.lib")

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${libs}
)