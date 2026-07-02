#!/usr/bin/env bash

# Detener el script si ocurre algún error
set -e

echo "========================================="
echo " Instalando dependencias en WSL..."
echo "========================================="

# 1. Actualizar las listas de paquetes
echo "[1/3] Actualizando repositorios locales..."
sudo apt update -y

# 2. Instalar herramientas de compilación y CMake
echo "[2/3] Instalar CMake, GCC, G++ y herramientas esenciales..."
sudo apt install -y build-essential cmake ninja-build

# 3. Verificar las instalaciones
echo "[3/3] Verificando versiones instaladas..."
echo "-----------------------------------------"
echo "CMake: $(cmake --version | head -n 1)"
echo "GCC:   $(gcc --version | head -n 1)"
echo "Ninja: $(ninja --version 2>/dev/null || echo 'No instalado, usando Unix Makefiles')"
echo "-----------------------------------------"

echo "¡Instalación completada con éxito!"
