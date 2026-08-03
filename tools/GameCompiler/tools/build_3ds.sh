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

# 1. Compilación del código base (Generar code.elf en build_3ds/code)
cd "$CODE_DIR" || exit 1
cmake ../.. -DBUILD_3DS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="${DEVKITPRO}/cmake/3DS.cmake"
cmake --build . || exit 1

# Copiar artefactos necesarios a la raíz de build_3ds (para que el empaquetador los encuentre estables)
cp code.elf "../code.elf"

# Volver a la raíz del proyecto para calcular la ruta al empaquetador de metadatos
cd "../.." || exit 1

# 2. Generación del ejecutable final (.3dsx usando variables dinámicas)
cd "$FINAL_DIR" || exit 1

cmake ../../../3ds \
  -DCODE="../../code.elf" \
  -DGAME_NAME="${VAL_GAME3DS}" \
  -DNAME="${VAL_TITLE}" \
  -DDESCRIPTION="${VAL_DESC}" \
  -DAUTHOR="${VAL_AUTHOR}" \
  -DCMAKE_TOOLCHAIN_FILE="${DEVKITPRO}/cmake/3DS.cmake"

cmake --build . || exit 1

# Mover el resultado final con su nombre dinámico a la raíz de build_3ds
cp "${VAL_GAME3DS}.3dsx" "../${VAL_GAME3DS}.3dsx"
cp "${VAL_GAME3DS}.cia" "../${VAL_GAME3DS}.cia"

echo "¡Compilación de 3DS finalizada con éxito!"
