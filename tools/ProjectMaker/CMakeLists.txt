cmake_minimum_required(VERSION 3.10)

project(ProjectMaker C CXX)

# ---------------- C++ ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------- SOURCES ----------------
set(SOURCES
    "${CMAKE_SOURCE_DIR}/../../src/pak_loader/pak_loader.cpp"
)

add_executable(${PROJECT_NAME} 
    main.cpp
    ${SOURCES}
    pak.rc
)

target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_PC)

target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CMAKE_SOURCE_DIR}/../../include"
)