#!/usr/bin/env bash

# Lista de directorios a eliminar
DIRECTORIES=(
  "build"
  "build_3ds"
  "tools/3DModelsConverter/build"
  "tools/LocalizationMaker/build"
  "tools/PakMaker/build"
  "tools/ProjectMaker/build"
  "tools/SoundMaker3DS/build"
)

echo "Limpiando directorios de construcción..."

for dir in "${DIRECTORIES[@]}"; do
  if [ -d "$dir" ]; then
    echo " -> Eliminando: $dir"
    rm -rf "$dir"
  fi
done

echo "Limpieza completada."
