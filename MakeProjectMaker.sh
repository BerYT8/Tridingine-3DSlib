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
mkdir -p "$LIB_DIR/include"
mkdir -p "$LIB_DIR/lib/pc"
mkdir -p "$LIB_DIR/lib/3ds"

# =========================
# Assets
# =========================
echo "[2/6] Copying engine assets..."

# Asegurar que existan los orígenes
mkdir -p "$ROOT/include"
mkdir -p "$ROOT/content"

# Copiar el contenido de las carpetas de forma limpia (evita rutas duplicadas)
cp -r "$ROOT/tools/GameCompiler/"* "$LIB_DIR/" 2>/dev/null
cp -r "$ROOT/include/"* "$LIB_DIR/include/" 2>/dev/null
cp -r "$ROOT/content/"* "$LIB_DIR/content/" 2>/dev/null

# =========================
# PC libs
# =========================
echo "[3/6] Copying PC libs..."

cp -f "$BUILD_DIR/Code/libTridingine.so" "$LIB_DIR/lib/pc/libTridingine.so" 2>/dev/null
cp -f "$BUILD_DIR/Code/libTridingineEntrypoint.a" "$LIB_DIR/lib/pc/libEntrypoint.a" 2>/dev/null

# =========================
# 3DS lib
# =========================
echo "[4/6] Copying 3DS libs..."

cp -f "$ROOT/build_3ds/libTridingine.a" "$LIB_DIR/lib/3ds/tridingine.a" 2>/dev/null
cp -f "$ROOT/build_3ds/libTridingineEntrypoint.a" "$LIB_DIR/lib/3ds/entrypoint.a" 2>/dev/null

# =========================
# Tools build
# =========================
echo "[5/6] Building tools..."

TOOLS=(
  "PakMaker"
  "3DModelsConverter"
  "LocalizationMaker"
  "FontsConverter"
)

for t in "${TOOLS[@]}"; do
  echo " -> $t"
  if [ -d "$ROOT/tools/$t" ]; then
    cmake -S "$ROOT/tools/$t" -B "$ROOT/tools/$t/build" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$ROOT/tools/$t/build"
    cp "$ROOT/tools/$t/build/$t" "$LIB_DIR/tools/" 2>/dev/null
  fi
done

# =========================
# SoundMaker3DS
# =========================
echo "Building SoundMaker3DS..."

SOUNDMAKER_DIR="$ROOT/tools/SoundMaker3DS"
OPUS_DIR="$SOUNDMAKER_DIR/libopus"
OPUSENC_DIR="$SOUNDMAKER_DIR/libopusenc"
SOUNDMAKER_BUILD="$SOUNDMAKER_DIR/build"
SOUNDMAKER_LIB="$SOUNDMAKER_DIR/lib"

mkdir -p "$SOUNDMAKER_LIB"

# =========================
# libopus
# =========================
echo "Building libopus..."

cmake -S "$OPUS_DIR" \
      -B "$OPUS_DIR/build" \
      -DOPUS_BUILD_PROGRAMS=ON \
      -DOPUS_BUILD_TESTING=ON \
      -DCMAKE_BUILD_TYPE=Release

cmake --build "$OPUS_DIR/build"

# Buscar libopus.a
if [ -f "$OPUS_DIR/build/libopus.a" ]; then
    cp -f "$OPUS_DIR/build/libopus.a" "$SOUNDMAKER_LIB/libopus.a"
elif [ -f "$OPUS_DIR/build/Release/libopus.a" ]; then
    cp -f "$OPUS_DIR/build/Release/libopus.a" "$SOUNDMAKER_LIB/libopus.a"
else
    echo "ERROR: libopus.a not found."
    exit 1
fi

echo "libopus.a copied successfully."

# =========================
# Preparar libopusenc
# =========================
echo "Preparing libopusenc..."

if [ ! -f "$SOUNDMAKER_DIR/cmake/libopusenc.cmake" ]; then
    echo "ERROR: libopusenc.cmake not found."
    exit 1
fi

cp -f \
    "$SOUNDMAKER_DIR/cmake/libopusenc.cmake" \
    "$OPUSENC_DIR/CMakeLists.txt"

# =========================
# Copiar headers de Opus
# =========================
echo "Copying Opus headers..."

mkdir -p "$OPUSENC_DIR/lib_include"

cp -f "$OPUS_DIR/include/"*.h "$OPUSENC_DIR/lib_include/"

# Comprobar que se copiaron
if ! ls "$OPUSENC_DIR/lib_include/"*.h >/dev/null 2>&1; then
    echo "ERROR: Opus headers were not copied."
    exit 1
fi

# =========================
# libopusenc
# =========================
echo "Building libopusenc..."

cmake -S "$OPUSENC_DIR" \
      -B "$OPUSENC_DIR/build" \
      -DCMAKE_BUILD_TYPE=Release

cmake --build "$OPUSENC_DIR/build"

# Buscar libopusenc.a
if [ -f "$OPUSENC_DIR/build/libopusenc.a" ]; then
    cp -f "$OPUSENC_DIR/build/libopusenc.a" "$SOUNDMAKER_LIB/libopusenc.a"
elif [ -f "$OPUSENC_DIR/build/Release/libopusenc.a" ]; then
    cp -f "$OPUSENC_DIR/build/Release/libopusenc.a" "$SOUNDMAKER_LIB/libopusenc.a"
else
    echo "ERROR: libopusenc.a not found."
    exit 1
fi

echo "libopusenc.a copied successfully."

# =========================
# SoundMaker3DS
# =========================
echo "Building SoundMaker3DS..."

cmake -S "$SOUNDMAKER_DIR" \
      -B "$SOUNDMAKER_BUILD" \
      -DCMAKE_BUILD_TYPE=Release

cmake --build "$SOUNDMAKER_BUILD"

# =========================
# Copiar ejecutable
# =========================
if [ -f "$SOUNDMAKER_BUILD/SoundMaker3DS" ]; then

    cp -f \
        "$SOUNDMAKER_BUILD/SoundMaker3DS" \
        "$LIB_DIR/tools/SoundMaker3DS"

elif [ -f "$SOUNDMAKER_BUILD/Release/SoundMaker3DS" ]; then

    cp -f \
        "$SOUNDMAKER_BUILD/Release/SoundMaker3DS" \
        "$LIB_DIR/tools/SoundMaker3DS"

else
    echo "ERROR: SoundMaker3DS executable not found."
    exit 1
fi

echo "SoundMaker3DS built successfully."

cd "$ROOT"

cd "$ROOT/tools/bannertool" && make || true
cd "$ROOT"

cp "$ROOT/tools/bannertool/output/linux-x86_64/bannertool" "$LIB_DIR/tools/" 2>/dev/null \
|| cp "$ROOT/tools/bannertool/output/linux-i686/bannertool" "$LIB_DIR/tools/" 2>/dev/null \
|| cp "$ROOT/tools/bannertool/output/macos/bannertool" "$LIB_DIR/tools/" 2>/dev/null \
|| cp "$ROOT/tools/bannertool/output/macos-x86_64/bannertool" "$LIB_DIR/tools/" 2>/dev/null \
|| cp "$ROOT/tools/bannertool/output/macos-arm64/bannertool" "$LIB_DIR/tools/" 2>/dev/null

cd "$ROOT/tools/Project_CTR/makerom"
make deps
make
cp "$ROOT/tools/Project_CTR/makerom/bin/makerom" "$LIB_DIR/tools/" 2>/dev/null

cmake -S "$ROOT/tools/3dstool" -B "$ROOT/tools/3dstool/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOT/tools/3dstool/build"
cp "$ROOT/tools/3dstool/bin/Release/3dstool" "$LIB_DIR/tools/" 2>/dev/null

# Generar el paquete PAK indispensable
"$LIB_DIR/tools/PakMaker" -c "$LIB_DIR" -o "$ROOT/tools/ProjectMaker/template.pak" -e "build" "build_3ds" "romfs" \
                          "examples"

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
cp "$ROOT/tools/ProjectMaker/build/ProjectMaker" "$BUILD_DIR/ProjectMaker" 2>/dev/null

echo "=== DONE ==="
