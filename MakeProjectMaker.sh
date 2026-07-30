#!/usr/bin/env bash
set -e

ROOT="$(pwd)"
BUILD_DIR="${ROOT}/build"
LIB_DIR="${BUILD_DIR}/Lib"

echo "=== ProjectMaker POST BUILD ==="

# =========================
# Crear estructura Lib
# =========================
echo "[1/6] Creating Lib structure..."

rm -rf "$LIB_DIR"
mkdir -p "$LIB_DIR"

mkdir -p "$LIB_DIR/tools"
mkdir -p "$LIB_DIR/content/game"
mkdir -p "$LIB_DIR/content/engine"
mkdir -p "$LIB_DIR/romfs/pc"
mkdir -p "$LIB_DIR/romfs/3ds"
mkdir -p "$LIB_DIR/templates"
mkdir -p "$LIB_DIR/examples"
mkdir -p "$LIB_DIR/include"
mkdir -p "$LIB_DIR/lib/pc"
mkdir -p "$LIB_DIR/lib/3ds"

# =========================
# Assets
# =========================
echo "[2/6] Copying engine assets..."

cp -r "$ROOT/tools/GameCompiler" "$LIB_DIR/" 2>/dev/null || true
cp -r "$ROOT/include" "$LIB_DIR/include"
cp -r "$ROOT/examples" "$LIB_DIR/examples"
cp -r "$ROOT/templates" "$LIB_DIR/templates"
cp -r "$ROOT/content" "$LIB_DIR/content"

# =========================
# PC libs
# =========================
echo "[3/6] Copying PC libs..."

cp -f "$BUILD_DIR/Code/Release/3ds_libs.a" "$LIB_DIR/lib/pc/" 2>/dev/null || true
cp -f "$ROOT/lib/x64/"*.so "$LIB_DIR/lib/pc/" 2>/dev/null || true
cp -f "$ROOT/lib/x64/"*.a "$LIB_DIR/lib/pc/" 2>/dev/null || true
cp -f "$ROOT/lib/glew/x64/glew32s.a" "$LIB_DIR/lib/pc/" 2>/dev/null || true

# =========================
# 3DS lib
# =========================
echo "[4/6] Copying 3DS libs..."

cp -f "$ROOT/build_3ds/code/lib3ds_libs.a" "$LIB_DIR/lib/3ds/3ds_libs.a" 2>/dev/null || true

# =========================
# Tools build
# =========================
echo "[5/6] Building tools..."

TOOLS=(
  "PakMaker"
  "3DModelsConverter"
  "SoundMaker3DS"
  "LocalizationMaker"
)

for t in "${TOOLS[@]}"; do
  echo " -> $t"
  if [ -d "$ROOT/tools/$t" ]; then
    cmake -S "$ROOT/tools/$t" -B "$ROOT/tools/$t/build" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$ROOT/tools/$t/build"
    cp "$ROOT/tools/$t/build/Release/"* "$LIB_DIR/tools/" 2>/dev/null || true
  fi
done

"$LIB_DIR/tools/PakMaker" -c "$LIB_DIR" -o "$ROOT/tools/ProjectMaker/pak.pak"

# =========================
# ProjectMaker
# =========================
echo "[6/6] Building ProjectMaker..."

cmake -S "$ROOT/tools/ProjectMaker" \
      -B "$ROOT/tools/ProjectMaker/build" \
      -DCMAKE_BUILD_TYPE=Release

cmake --build "$ROOT/tools/ProjectMaker/build"

cp "$ROOT/tools/ProjectMaker/build/Release/ProjectMaker" "$ROOT/ProjectMaker" 2>/dev/null || \
cp "$ROOT/tools/ProjectMaker/build/Release/ProjectMaker.exe" "$ROOT/ProjectMaker" 2>/dev/null || true

echo "=== DONE ==="