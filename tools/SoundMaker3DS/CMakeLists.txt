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

target_link_libraries(${PROJECT_NAME} PRIVATE opusenc opus)

target_include_directories(${PROJECT_NAME} PRIVATE 
    lib_include
    include
)

# ---------------- SELECCIÓN DE LIBRERÍAS POR PLATAFORMA ----------------
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Si es Windows nativo o compilación cruzada para Windows, busca archivos .lib
    file(GLOB_RECURSE libs "libs/*.lib")
else()
    # Si es Linux, WSL o macOS, busca archivos .a o .so
    file(GLOB_RECURSE libs "libs/*.a" "libs/*.so")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${libs}
)
