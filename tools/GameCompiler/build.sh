#!/bin/bash

# Crear directorios base si no existen
mkdir -p romfs romfs/3ds romfs/pc content content/engine content/game

chmod +x ./tools/3DModelsConverter
chmod +x ./tools/SoundMaker3DS
chmod +x ./tools/LocalizationMaker
chmod +x ./tools/PakMaker

if [ "$1" == "3ds" ]; then
    echo "Iniciando compilacion para 3DS..."
    chmod +x ./tools/build_3ds.sh
    ./tools/build_3ds.sh
    exit 0
fi

# Compilación por defecto (PC / Nativo)
echo "Compilando version para PC..."
BUILD_DIR="build"
CONTENT_DIR="content"
ROMFS_DIR="romfs/pc"
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR" || exit 1
cmake ..
cmake --build . --config Release
cp ../lib/pc/*.so Release/ 2>/dev/null
cd ..

./tools/3DModelsConverter --all -i "$CONTENT_DIR" -o "$ROMFS_DIR"
./tools/SoundMaker3DS --all -i "$CONTENT_DIR" -o "$ROMFS_DIR"
./tools/LocalizationMaker --all -i "$CONTENT_DIR" -o "$ROMFS_DIR"

./tools/PakMaker -c "$CONTENT_DIR" -o "$BUILD_DIR/game.pak"