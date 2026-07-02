cmake_minimum_required(VERSION 3.10)

project(PakMaker C CXX)

# ---------------- C++ ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------- SOURCES ----------------
set(SOURCES
    # Añade aquí más fuentes si es necesario
)

add_executable(${PROJECT_NAME} 
    ${SOURCES}
    main.cpp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_PC)

target_include_directories(${PROJECT_NAME} PRIVATE 
)

# ---------------- SELECCIÓN DE LIBRERÍAS POR PLATAFORMA ----------------
if(WIN32)
    # Si es Windows, busca archivos .lib
    file(GLOB_RECURSE libs "lib/x64/*.lib")
else()
    # Si es Linux / WSL / macOS, busca archivos .a o .so
    file(GLOB_RECURSE libs "lib/x64/*.a" "lib/x64/*.so")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${libs}
)
