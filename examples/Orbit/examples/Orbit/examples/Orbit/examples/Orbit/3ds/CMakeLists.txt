cmake_minimum_required(VERSION 3.10)

project(game C CXX)

if(NOT DEFINED GAME_NAME)
    # Asigna un valor por defecto si no se introduce en la terminal
    set(GAME_NAME ${PROJECT_NAME} CACHE STRING "Nombre del proyecto 3DS")
endif()

if(NOT DEFINED NAME)
    if(DEFINED PROJECT_NAME)
        set(NAME ${PROJECT_NAME} CACHE STRING "Nombre del proyecto 3DS")
    else()
        set(NAME "MiJuego3DS" CACHE STRING "Nombre del proyecto 3DS")
    endif()
endif()

if(NOT DEFINED DESCRIPTION)
    # Asigna un valor por defecto si no se introduce en la terminal
    set(DESCRIPTION "MiJuego3DS" CACHE STRING "Descripción del proyecto 3DS")
endif()

if(NOT DEFINED AUTHOR)
    # Asigna un valor por defecto si no se introduce en la terminal
    set(AUTHOR "Yo" CACHE STRING "Autor del proyecto 3DS")
endif()

if(NOT DEFINED CODE)
    # Asigna un valor por defecto si no se introduce en la terminal
    set(CODE "code.elf" CACHE STRING "Código del proyecto 3DS")
endif()

# ---------------- C++ ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mi_target_externo IMPORTED)
set_property(TARGET mi_target_externo PROPERTY IMPORTED_LOCATION ${CODE})
    message(STATUS "ROMFS: ${CMAKE_CURRENT_SOURCE_DIR}/../romfs/3ds")

ctr_generate_smdh(
    OUTPUT "${GAME_NAME}.smdh"
    NAME ${NAME}
    DESCRIPTION ${DESCRIPTION}
    AUTHOR ${AUTHOR}
)

ctr_create_3dsx(${GAME_NAME}
    TARGET mi_target_externo
    SMDH "${GAME_NAME}.smdh"
    ROMFS "${CMAKE_SOURCE_DIR}/../romfs/3ds"
)