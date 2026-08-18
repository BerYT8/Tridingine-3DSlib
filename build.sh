#!/bin/bash

# Detener el script si ocurre algún error
set -e

BUILD_DIR="build"

cd external/glew

make -C auto
make glew.lib.static

cd ../..

# Crear directorio de construcción si no existe
mkdir -p "$BUILD_DIR"

# =========================
# PC BUILD
# =========================
cd "$BUILD_DIR"

# Limpiar y recrear directorios
rm -rf Lib
mkdir -p Lib
mkdir -p Code

cd Code
cmake ../.. -DBUILD_3DS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
# sudo cmake --install .

cd ../..

cp build/Code/compile_commands.json compile_commands.json

# =========================
# 3DS BUILD (Llamada Directa)
# =========================
echo "Building 3DS..."

# Verifica si tienes las variables de entorno de devkitPro (opcional pero recomendado)
if [ -z "$DEVKITPRO" ]; then
    echo "WARNING: La variable DEVKITPRO no está definida. El build podría fallar."
fi

# Llamada normal y directa al script de la 3DS
chmod +x ./build_3ds.sh
./build_3ds.sh build_3ds

echo "Done."
