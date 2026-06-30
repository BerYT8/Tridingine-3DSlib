#!/bin/bash

cd ..

CURRENT_DIR=$(pwd)

BUILD_DIR="$1"

if [ -z "$BUILD_DIR" ]; then
    BUILD_DIR="build_3ds/code"
fi

echo "Using build directory: $BUILD_DIR"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || { echo "Failed to enter build directory."; exit 1; }

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
