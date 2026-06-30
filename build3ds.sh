#!/bin/bash

CURRENT_DIR=$(pwd)

# Guardar argumento: carpeta de build
BUILD_DIR="$1"

# Si no se pasó argumento, usar "build_3ds" por defecto
if [ -z "$BUILD_DIR" ]; then
    BUILD_DIR="build_3ds/code"
fi

echo "Using build directory: $BUILD_DIR"

# Crear carpeta si no existe
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || { echo "Failed to enter build directory."; exit 1; }

# Configuración y build
/opt/devkitpro/msys2/usr/bin/cmake.exe ../.. -DBUILD_3DS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="${DEVKITPRO}/cmake/3DS.cmake"
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
