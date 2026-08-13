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
  "tools/3dstool/build"
  "tools/3dstool/bin"
  "tools/bannertool/build"
  "tools/bannertool/output"
  "tools/Project_CTR/ctrtool/build"
  "tools/Project_CTR/makerom/build"
  "tools/Project_CTR/makerom/bin"
)

echo "Limpiando directorios de construcción..."

for dir in "${DIRECTORIES[@]}"; do
  if [ -d "$dir" ]; then
    echo " -> Eliminando: $dir"
    rm -rf "$dir"
  fi
done

echo "Limpieza completada."
