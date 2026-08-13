#!/bin/bash
# Ir a la raíz del proyecto desde la carpeta tools
cd "$(dirname "$0")/.." || exit 1

# Variables por defecto
VAL_GAME3DS="Game3DS"
VAL_TITLE="Juego 3DS"
VAL_DESC="Juego hecho con 3DSLIB."
VAL_AUTHOR="Tú"

# Leer JSON si existe (usando herramientas nativas de bash/sed/grep para evitar dependencias)
if [ -f "game.json" ]; then
    echo "Leyendo datos desde game.json..."

    VAL_GAME3DS=$(grep -o '"file": "[^"]*' game.json | grep -o '[^"]*$')
    VAL_TITLE=$(grep -o '"title": "[^"]*' game.json | grep -o '[^"]*$')
    VAL_AUTHOR=$(grep -o '"author": "[^"]*' game.json | grep -o '[^"]*$')
    VAL_DESC=$(grep -o '"description": "[^"]*' game.json | grep -o '[^"]*$')

    # Reemplazar espacios por guiones bajos en el nombre del archivo
    VAL_GAME3DS="${VAL_GAME3DS// /_}"
fi

# Ajustar valores vacíos a los por defecto
VAL_GAME3DS=${VAL_GAME3DS:-Game3DS}
VAL_TITLE=${VAL_TITLE:-Juego 3DS}
VAL_DESC=${VAL_DESC:-Juego hecho con 3DSLIB.}
VAL_AUTHOR=${VAL_AUTHOR:-Tú}

# Configurar directorios de compilación fijos para reutilizar caché
BUILD_DIR="build_3ds"
CODE_DIR="${BUILD_DIR}/code"
FINAL_DIR="${BUILD_DIR}/${VAL_GAME3DS}/compiled_game"

mkdir -p "$CODE_DIR"
mkdir -p "$FINAL_DIR"

# ============================================================
# 1. Compilación del código base
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
  -DCMAKE_TOOLCHAIN_FILE="${DEVKITPRO}/cmake/3DS.cmake"

cmake --build . || exit 1

# Copiar code.elf a la raíz de build_3ds
cp code.elf "../code.elf"

# Volver a la raíz del proyecto
cd "../.." || exit 1

# ============================================================
# 2. Generación del ejecutable final
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

# Mover los resultados a la raíz de build_3ds
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
# 3. Link opcional a Nintendo 3DS
# ============================================================

if [ "$1" = "link" ]; then

    # Eliminar "link" de los argumentos.
    # Todo lo que quede se pasa directamente a 3dslink.
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

    # Comprobar que 3dslink existe
    if ! command -v 3dslink >/dev/null 2>&1; then
        echo "ERROR: No se encontró '3dslink'."
        echo "Comprueba que devkitPro/3dslink está instalado y disponible en PATH."
        exit 1
    fi

    # Ejecutar 3dslink.
    # "$@" permite pasar cualquier argumento adicional.
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
