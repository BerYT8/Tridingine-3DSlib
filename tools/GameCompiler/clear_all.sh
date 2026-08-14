#!/usr/bin/env bash

# Lista de directorios a eliminar
DIRECTORIES=(
  "build"
  "build_3ds"
  "romfs"
)

echo "Limpiando directorios de construcción..."

for dir in "${DIRECTORIES[@]}"; do
  if [ -d "$dir" ]; then
    echo " -> Eliminando: $dir"
    rm -rf "$dir"
  fi
done

echo "Limpieza completada."
