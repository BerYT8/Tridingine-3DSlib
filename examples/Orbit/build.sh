#!/bin/bash

# Crear directorios base si no existen
mkdir -p romfs romfs/3ds romfs/pc content content/engine content/game

chmod +x ./tools/FontsConverter
chmod +x ./tools/3DModelsConverter
chmod +x ./tools/SoundMaker3DS
chmod +x ./tools/LocalizationMaker
chmod +x ./tools/PakMaker
chmod +x ./tools/bannertool
chmod +x ./tools/makerom

CONTENT_DIR="content"
ROMFS_DIR_3DS="romfs/3ds"

if [ "$1" == "3ds" ]; then
    echo "Iniciando compilacion para 3DS..."
    chmod +x ./tools/build_3ds.sh

    ./tools/FontsConverter --all -3ds -i "$CONTENT_DIR" -o "$ROMFS_DIR_3DS"
    ./tools/3DModelsConverter --all -i "$CONTENT_DIR" -o "$ROMFS_DIR_3DS"
    ./tools/SoundMaker3DS --all -i "$CONTENT_DIR" -o "$ROMFS_DIR_3DS"
    ./tools/LocalizationMaker --all -i "$CONTENT_DIR" -o "$ROMFS_DIR_3DS"

    mkdir -p "$ROMFS_DIR_3DS/certs"
    cp "$CONTENT_DIR/certs/ca-bundle.crt" "$ROMFS_DIR_3DS/certs/ca-bundle.crt" || exit 1

    ./tools/build_3ds.sh "${@:2}"
    exit 0
fi

# Compilación por defecto (PC / Nativo)
echo "Compilando version para PC..."
BUILD_DIR="build"
ROMFS_DIR="romfs/pc"
mkdir -p "$BUILD_DIR"

# --- LEER DATOS DESDE GAME.JSON CON GREP (NATIVO) ---
CMAKE_FLAGS=()
JSON_FILE="game.json"

if [ -f "$JSON_FILE" ]; then
    # Extrae el valor de "title"
    TITLE=$(grep -o '"title": "[^"]*' "$JSON_FILE" | grep -o '[^"]*$')
    if [ -n "$TITLE" ]; then
        echo "Título detectado en $JSON_FILE: '$TITLE'"
        CMAKE_FLAGS+=(-DGAME_TITLE="$TITLE")
    fi

    # Extrae el valor de "file"
    GAME_FILE=$(grep -o '"file": "[^"]*' "$JSON_FILE" | grep -o '[^"]*$')
    if [ -n "$GAME_FILE" ]; then
        # Reemplazar espacios por guiones bajos
        GAME_FILE="${GAME_FILE// /_}"

        echo "Nombre de archivo detectado en $JSON_FILE: '$GAME_FILE'"
        CMAKE_FLAGS+=(-DGAME_NAME="$GAME_FILE")
    fi

    # Extrae el valor de "author"
    AUTHOR=$(grep -o '"author": "[^"]*' "$JSON_FILE" | grep -o '[^"]*$')
    if [ -n "$AUTHOR" ]; then
        echo "Autor detectado en $JSON_FILE: '$AUTHOR'"
        CMAKE_FLAGS+=(-DGAME_AUTHOR="$AUTHOR")
    fi

    # Extrae el valor de "description"
    DESCRIPTION=$(grep -o '"description": "[^"]*' "$JSON_FILE" | grep -o '[^"]*$')
    if [ -n "$DESCRIPTION" ]; then
        echo "Descripción detectada en $JSON_FILE: '$DESCRIPTION'"
        CMAKE_FLAGS+=(-DGAME_DESC="$DESCRIPTION")
    fi

    # Extrae los sources
    SOURCES=$(
        sed -n '/"sources"[[:space:]]*:/,/]/p' "$JSON_FILE" \
        | tail -n +2 \
        | grep -o '"[^"]*"' \
        | sed 's/"//g' \
        | tr '\n' ';'
    )

    if [ -n "$SOURCES" ]; then
        SOURCES="${SOURCES%;}"

        echo "Sources detectados en $JSON_FILE:"
        echo "  $SOURCES"

        CMAKE_FLAGS+=(-DGAME_SOURCES="$SOURCES")
    fi

    # Extrae los includes
    INCLUDES=$(
        sed -n '/"includes"[[:space:]]*:/,/]/p' "$JSON_FILE" \
        | tail -n +2 \
        | grep -o '"[^"]*"' \
        | sed 's/"//g' \
        | tr '\n' ';'
    )

    if [ -n "$INCLUDES" ]; then
        INCLUDES="${INCLUDES%;}"

        echo "Includes detectados en $JSON_FILE:"
        echo "  $INCLUDES"

        CMAKE_FLAGS+=(-DGAME_INCLUDES="$INCLUDES")
    fi

fi

# -----------------------------------------------------

cd "$BUILD_DIR" || exit 1

# Al usar "${CMAKE_FLAGS[@]}", Bash expande todas las flags añadidas con sus espacios protegidos
cmake .. "${CMAKE_FLAGS[@]}"
cmake --build . --config Release
cp ../lib/pc/*.so Release/ 2>/dev/null
cd ..

./tools/FontsConverter --all -pc -i "$CONTENT_DIR" -o "$ROMFS_DIR"
./tools/3DModelsConverter --all -i "$CONTENT_DIR" -o "$ROMFS_DIR"
./tools/SoundMaker3DS --all -i "$CONTENT_DIR" -o "$ROMFS_DIR"
./tools/LocalizationMaker --all -i "$CONTENT_DIR" -o "$ROMFS_DIR"

mkdir -p "$ROMFS_DIR/certs"
cp "$CONTENT_DIR/certs/ca-bundle.crt" "$ROMFS_DIR/certs/ca-bundle.crt" || exit 1

./tools/PakMaker -c "$ROMFS_DIR" -o "$BUILD_DIR/game.pak"

cp build/compile_commands.json compile_commands.json