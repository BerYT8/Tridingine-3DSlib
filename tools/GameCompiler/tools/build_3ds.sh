#!/bin/bash

# Ir a la raíz del proyecto desde la carpeta tools
cd "$(dirname "$0")/.." || exit 1

# ============================================================
# VALORES POR DEFECTO
# ============================================================

VAL_GAME3DS="Game3DS"
VAL_TITLE="Juego 3DS"
VAL_DESC="Juego hecho con 3DSLIB."
VAL_AUTHOR="Tú"

GAME_SOURCES="main.cpp"
GAME_INCLUDES=""

JSON_FILE="game.json"

# ============================================================
# LEER GAME.JSON
# ============================================================

if [ -f "$JSON_FILE" ]; then

    echo "Leyendo datos desde $JSON_FILE..."

    # --------------------------------------------------------
    # File
    # --------------------------------------------------------

    VAL_GAME3DS=$(grep -o '"file": "[^"]*' "$JSON_FILE" | grep -o '[^"]*$')

    if [ -n "$VAL_GAME3DS" ]; then
        VAL_GAME3DS="${VAL_GAME3DS// /_}"

        echo "Nombre de archivo detectado: '$VAL_GAME3DS'"
    fi

    # --------------------------------------------------------
    # Title
    # --------------------------------------------------------

    VAL_TITLE=$(grep -o '"title": "[^"]*' "$JSON_FILE" | grep -o '[^"]*$')

    if [ -n "$VAL_TITLE" ]; then
        echo "Título detectado: '$VAL_TITLE'"
    fi

    # --------------------------------------------------------
    # Author
    # --------------------------------------------------------

    VAL_AUTHOR=$(grep -o '"author": "[^"]*' "$JSON_FILE" | grep -o '[^"]*$')

    if [ -n "$VAL_AUTHOR" ]; then
        echo "Autor detectado: '$VAL_AUTHOR'"
    fi

    # --------------------------------------------------------
    # Description
    # --------------------------------------------------------

    VAL_DESC=$(grep -o '"description": "[^"]*' "$JSON_FILE" | grep -o '[^"]*$')

    if [ -n "$VAL_DESC" ]; then
        echo "Descripción detectada: '$VAL_DESC'"
    fi

    # --------------------------------------------------------
    # Sources
    # --------------------------------------------------------

    SOURCES=$(
        sed -n '/"sources"[[:space:]]*:/,/]/p' "$JSON_FILE" \
        | tail -n +2 \
        | grep -o '"[^"]*"' \
        | sed 's/"//g' \
        | tr '\n' ';'
    )

    if [ -n "$SOURCES" ]; then

        SOURCES="${SOURCES%;}"

        echo "Sources detectados:"
        echo "  $SOURCES"

        GAME_SOURCES="$SOURCES"

    fi

    # --------------------------------------------------------
    # Includes
    # --------------------------------------------------------

    INCLUDES=$(
        sed -n '/"includes"[[:space:]]*:/,/]/p' "$JSON_FILE" \
        | tail -n +2 \
        | grep -o '"[^"]*"' \
        | sed 's/"//g' \
        | tr '\n' ';'
    )

    if [ -n "$INCLUDES" ]; then

        INCLUDES="${INCLUDES%;}"

        echo "Includes detectados:"
        echo "  $INCLUDES"

        GAME_INCLUDES="$INCLUDES"

    fi

fi

# ============================================================
# APLICAR VALORES POR DEFECTO
# ============================================================

VAL_GAME3DS=${VAL_GAME3DS:-Game3DS}
VAL_TITLE=${VAL_TITLE:-Juego 3DS}
VAL_DESC=${VAL_DESC:-Juego hecho con 3DSLIB.}
VAL_AUTHOR=${VAL_AUTHOR:-Tú}
GAME_SOURCES=${GAME_SOURCES:-main.cpp}

# ============================================================
# MOSTRAR CONFIGURACIÓN
# ============================================================

echo ""
echo "========================================"
echo " Configuración del juego"
echo "========================================"
echo " Game name : ${VAL_GAME3DS}"
echo " Title     : ${VAL_TITLE}"
echo " Author    : ${VAL_AUTHOR}"
echo " Desc      : ${VAL_DESC}"
echo " Sources   : ${GAME_SOURCES}"
echo " Includes  : ${GAME_INCLUDES}"
echo "========================================"
echo ""

# ============================================================
# DIRECTORIOS DE COMPILACIÓN
# ============================================================

BUILD_DIR="build_3ds"
CODE_DIR="${BUILD_DIR}/code"
FINAL_DIR="${BUILD_DIR}/${VAL_GAME3DS}/compiled_game"

mkdir -p "$CODE_DIR"
mkdir -p "$FINAL_DIR"

# ============================================================
# 1. COMPILACIÓN DEL CÓDIGO BASE
# ============================================================

echo ""
echo "========================================"
echo " Compilando código 3DS"
echo "========================================"
echo ""

cd "$CODE_DIR" || exit 1

cmake ../.. \
    -DBUILD_3DS=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="${DEVKITPRO}/cmake/3DS.cmake" \
    -DGAME_TITLE="${VAL_TITLE}" \
    -DGAME_DESCRIPTION="${VAL_DESC}" \
    -DGAME_AUTHOR="${VAL_AUTHOR}" \
    -DGAME_SOURCES="${GAME_SOURCES}" \
    -DGAME_INCLUDES="${GAME_INCLUDES}"

cmake --build . || exit 1

# Copiar code.elf a la raíz de build_3ds
cp code.elf "../code.elf"

# Volver a la raíz del proyecto
cd "../.." || exit 1

# ============================================================
# 2. GENERACIÓN DEL EJECUTABLE FINAL
# ============================================================

echo ""
echo "========================================"
echo " Generando .3dsx y .cia"
echo "========================================"
echo ""

cd "$FINAL_DIR" || exit 1

cmake ../../../3ds \
    -DCODE="../../code.elf" \
    -DGAME_NAME="${VAL_GAME3DS}" \
    -DNAME="${VAL_TITLE}" \
    -DDESCRIPTION="${VAL_DESC}" \
    -DAUTHOR="${VAL_AUTHOR}" \
    -DCMAKE_TOOLCHAIN_FILE="${DEVKITPRO}/cmake/3DS.cmake"

cmake --build . || exit 1

# Mover resultados a la raíz de build_3ds
cp "${VAL_GAME3DS}.3dsx" "../${VAL_GAME3DS}.3dsx"
cp "${VAL_GAME3DS}.cia" "../${VAL_GAME3DS}.cia"

# Volver a la raíz del proyecto
cd ../../.. || exit 1

echo ""
echo "========================================"
echo " ¡Compilación de 3DS finalizada!"
echo "========================================"
echo ""
echo "3DSX: ${BUILD_DIR}/${VAL_GAME3DS}/${VAL_GAME3DS}.3dsx"
echo "CIA:  ${BUILD_DIR}/${VAL_GAME3DS}/${VAL_GAME3DS}.cia"
echo ""

# ============================================================
# 3. LINK OPCIONAL A NINTENDO 3DS
# ============================================================

if [ "$1" = "link" ]; then

    shift

    GAME_3DSX="${BUILD_DIR}/${VAL_GAME3DS}/${VAL_GAME3DS}.3dsx"

    echo ""
    echo "========================================"
    echo " Enviando juego a Nintendo 3DS"
    echo "========================================"
    echo ""
    echo "Archivo: ${GAME_3DSX}"

    if [ "$#" -gt 0 ]; then
        echo "Argumentos 3dslink: $*"
    else
        echo "Sin argumentos adicionales (autodetección)"
    fi

    echo ""

    if ! command -v 3dslink >/dev/null 2>&1; then
        echo "ERROR: No se encontró '3dslink'."
        echo "Comprueba que devkitPro/3dslink está instalado y disponible en PATH."
        exit 1
    fi

    3dslink "$GAME_3DSX" "$@"

    LINK_RESULT=$?

    if [ $LINK_RESULT -ne 0 ]; then
        echo ""
        echo "ERROR: 3dslink terminó con código ${LINK_RESULT}."
        exit $LINK_RESULT
    fi

    echo ""
    echo "Juego enviado correctamente a la Nintendo 3DS."

fi
