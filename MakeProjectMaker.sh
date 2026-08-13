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

# Asegurar que existan los orígenes
mkdir -p "$ROOT/include"
mkdir -p "$ROOT/examples"
mkdir -p "$ROOT/templates"
mkdir -p "$ROOT/content"

# Copiar el contenido de las carpetas de forma limpia (evita rutas duplicadas)
cp -r "$ROOT/tools/GameCompiler/"* "$LIB_DIR/" 2>/dev/null || true
cp -r "$ROOT/include/"* "$LIB_DIR/include/" 2>/dev/null || true
cp -r "$ROOT/examples/"* "$LIB_DIR/examples/" 2>/dev/null || true
cp -r "$ROOT/templates/"* "$LIB_DIR/templates/" 2>/dev/null || true
cp -r "$ROOT/content/"* "$LIB_DIR/content/" 2>/dev/null || true

# =========================
# PC libs
# =========================
echo "[3/6] Copying PC libs..."

cp -f "$BUILD_DIR/Code/lib3ds_libs.so" "$LIB_DIR/lib/pc/lib3ds_libs.so" 2>/dev/null || true

# =========================
# 3DS lib
# =========================
echo "[4/6] Copying 3DS libs..."

cp -f "$ROOT/build_3ds/lib3ds_libs.a" "$LIB_DIR/lib/3ds/3ds_libs.a" 2>/dev/null || true

# =========================
# Tools build
# =========================
echo "[5/6] Building tools..."

TOOLS=(
  "PakMaker"
  "3DModelsConverter"
  "SoundMaker3DS"
  "LocalizationMaker"
  "FontsConverter"
)

for t in "${TOOLS[@]}"; do
  echo " -> $t"
  if [ -d "$ROOT/tools/$t" ]; then
    cmake -S "$ROOT/tools/$t" -B "$ROOT/tools/$t/build" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$ROOT/tools/$t/build"
    cp "$ROOT/tools/$t/build/$t" "$LIB_DIR/tools/" 2>/dev/null || true
  fi
done

cd "$ROOT/tools/bannertool" && make || true
cd "$ROOT"

cp "$ROOT/tools/bannertool/output/linux-x86_64/bannertool" "$LIB_DIR/tools/" 2>/dev/null \
|| cp "$ROOT/tools/bannertool/output/linux-i686/bannertool" "$LIB_DIR/tools/" 2>/dev/null \
|| cp "$ROOT/tools/bannertool/output/macos/bannertool" "$LIB_DIR/tools/" 2>/dev/null \
|| cp "$ROOT/tools/bannertool/output/macos-x86_64/bannertool" "$LIB_DIR/tools/" 2>/dev/null \
|| cp "$ROOT/tools/bannertool/output/macos-arm64/bannertool" "$LIB_DIR/tools/" 2>/dev/null \
|| true

cd "$ROOT/tools/Project_CTR/makerom"
make deps
make
cp "$ROOT/tools/Project_CTR/makerom/bin/makerom" "$LIB_DIR/tools/" 2>/dev/null || true

cmake -S "$ROOT/tools/3dstool" -B "$ROOT/tools/3dstool/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOT/tools/3dstool/build"
cp "$ROOT/tools/3dstool/bin/Release/3dstool" "$LIB_DIR/tools/" 2>/dev/null || true

# Generar el paquete PAK indispensable
"$LIB_DIR/tools/PakMaker" -c "$LIB_DIR" -o "$ROOT/tools/ProjectMaker/pak.pak" -e "build" "build_3ds" "romfs" "examples/*/examples/*"

# Copia de seguridad del PAK para el entorno Linux modificado
mkdir -p "$ROOT/tools/ProjectMaker/build"

# =========================
# ProjectMaker
# =========================
echo "[6/6] Building ProjectMaker..."

cmake -S "$ROOT/tools/ProjectMaker" \
      -B "$ROOT/tools/ProjectMaker/build" \
      -DCMAKE_BUILD_TYPE=Release

cmake --build "$ROOT/tools/ProjectMaker/build"

# Corregido el cierre del comando de copia colgante
cp "$ROOT/tools/ProjectMaker/build/ProjectMaker" "$BUILD_DIR/ProjectMaker" 2>/dev/null || true

echo "=== DONE ==="
