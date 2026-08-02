cmake_minimum_required(VERSION 3.12)
project(FontsConverter CXX)

# Requerir C++17 para usar std::filesystem
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar dependencias mediante PkgConfig
find_package(PkgConfig REQUIRED)
pkg_check_modules(FREETYPE REQUIRED freetype2)
pkg_check_modules(IMAGEMAGICK REQUIRED Magick++)

# Configurar el ejecutable
add_executable(${PROJECT_NAME}
    main.cpp
    source/bcfnt.cpp
    source/freetype.cpp
    source/magick_compat.cpp
    source/swizzle.cpp
    source/threadPool.cpp
)

# Configurar directorios de inclusión (include)
target_include_directories(${PROJECT_NAME} PRIVATE 
    include
    ${FREETYPE_INCLUDE_DIRS}
    ${IMAGEMAGICK_INCLUDE_DIRS}
)

# Vincular las librerías necesarias
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${FREETYPE_LIBRARIES}
    ${IMAGEMAGICK_LIBRARIES}
)

# Pasar banderas de compilación del sistema si existen
target_compile_options(${PROJECT_NAME} PRIVATE 
    ${FREETYPE_CFLAGS_OTHER}
    ${IMAGEMAGICK_CFLAGS_OTHER}
)
