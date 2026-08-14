#!/bin/bash

set -e

BUILD_DIR="${1:-build_3ds}"

echo "=== 3DS BUILD ==="
echo "Dir: $BUILD_DIR"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# =========================
# SHADERS (Picasso)
# =========================
echo "[Shaders] Compiling..."

TOOLS="$DEVKITPRO/tools/bin"
ARM_AS="$DEVKITPRO/devkitARM/bin/arm-none-eabi-as"

PICA="../src/draw/3d/3dshader.v.pica"
HEADER="../src/draw/3d/3dshader_shbin.h"

SHBIN="3dshader.shbin"
ASM="3dshader.s"

"$TOOLS/picasso" "$PICA" -o "$SHBIN"
"$TOOLS/bin2s" "$SHBIN" -H "$HEADER"

echo "[Shaders] OK"

# =========================
# CMAKE 3DS
# =========================
echo "[CMake] Configuring..."

cmake .. \
  -DBUILD_3DS=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/3DS.cmake" \
  -DCMAKE_INSTALL_PREFIX="$DEVKITPRO/portlibs/3ds"

echo "[CMake] Building..."
cmake --build .
sudo cmake --install .

echo "=== 3DS BUILD DONE ==="