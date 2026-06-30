cmake_minimum_required(VERSION 3.10)

project(3DModelsConverter C CXX)

# ---------------- C++ ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------- SOURCES ----------------
set(SOURCES
    
)

add_executable(${PROJECT_NAME} 
    ${SOURCES}
    main.cpp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_PC)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_SOURCE_DIR}/../../include
    ${CMAKE_SOURCE_DIR}/../../src
)

file(GLOB_RECURSE libs "lib/x64/*.lib")

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${libs}
)