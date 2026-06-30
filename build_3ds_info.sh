#!/bin/bash

CURRENT_DIR=$(pwd)

# Guardar argumento: carpeta de build
BUILD_DIR="$1"

# Si no se pasó argumento, usar "build_3ds" por defecto
if [ -z "$BUILD_DIR" ]; then
    BUILD_DIR="build_3ds/compiled_game"
fi

echo "Using build directory: $BUILD_DIR"

# Crear carpeta si no existe
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || { echo "Failed to enter build directory."; exit 1; }

CODE="$2"

# Si no se pasó argumento, usar "build_3ds" por defecto
if [ -z "$CODE" ]; then
    CODE="code.elf"
fi

GAME_NAME="$3"

# Si no se pasó argumento, usar "build_3ds" por defecto
if [ -z "$GAME_NAME" ]; then
    GAME_NAME="Game"
fi

NAME="$4"

# Si no se pasó argumento, usar "build_3ds" por defecto
if [ -z "$NAME" ]; then
    NAME="Game"
fi

DESC="$5"

# Si no se pasó argumento, usar "build_3ds" por defecto
if [ -z "$DESC" ]; then
    DESC="Description"
fi

AUTHOR="$6"

# Si no se pasó argumento, usar "build_3ds" por defecto
if [ -z "$AUTHOR" ]; then
    AUTHOR="Author"
fi

# Configuración y build
/opt/devkitpro/msys2/usr/bin/cmake.exe ../../../3ds \
  -DCODE="${CODE}" \
  -DGAME_NAME="${GAME_NAME}" \
  -DNAME="${NAME}" \
  -DDESCRIPTION="${DESC}" \
  -DAUTHOR="${AUTHOR}" \
  -DCMAKE_TOOLCHAIN_FILE="${DEVKITPRO}/cmake/3DS.cmake"

if [ $? -ne 0 ]; then
    echo "CMake configuration failed."
    exit 1
fi

/opt/devkitpro/msys2/usr/bin/cmake.exe --build .
if [ $? -ne 0 ]; then
    echo "Build failed."
    exit 1
fi

echo "Build finished successfully."
