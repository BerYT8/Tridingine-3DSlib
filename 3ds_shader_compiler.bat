@echo off

:: ----------------------------------------------------
:: CONFIGURACIÓN DE RUTA DE PROYECTO Y HERRAMIENTAS
:: ----------------------------------------------------
:: Cambia estas rutas si tus carpetas están en otro lugar
set "CMAKE_CURRENT_SOURCE_DIR=."
set "CMAKE_CURRENT_BINARY_DIR=%CMAKE_CURRENT_SOURCE_DIR%\build_3ds\code"

set "TOOLS_SRC=C:\devkitPro\tools\bin"
set "ARM_COMP=C:\devkitPro\devkitARM\bin\arm-none-eabi-as"

:: Definimos las variables de los archivos protegiendo los espacios
set "HEADER=%CMAKE_CURRENT_SOURCE_DIR%\src\draw\3d\3dshader_shbin.h"
set "SHADER_PICA=%CMAKE_CURRENT_SOURCE_DIR%\src\draw\3d\3dshader.v.pica"
set "SHADER_SHBIN=%CMAKE_CURRENT_BINARY_DIR%\3dshader_shbin.shbin"
set "SHADER_S=%CMAKE_CURRENT_BINARY_DIR%\3dshader_shbin.s"
set "SHADER_OBJ=%CMAKE_CURRENT_BINARY_DIR%\3dshader_shbin.o"

:: ----------------------------------------------------
:: EJECUCIÓN DE LOS PASOS DE COMPILACIÓN
:: ----------------------------------------------------

echo [1/4] Picasso: Compilando shader .v.pica...
"%TOOLS_SRC%\picasso.exe" "%SHADER_PICA%" -o "%SHADER_SHBIN%"
if %errorlevel% neq 0 (echo Error en Picasso & pause & exit /b %errorlevel%)

echo [2/4] bin2s: Generando cabecera de shader .h...
"%TOOLS_SRC%\bin2s.exe" "%SHADER_SHBIN%" -H "%HEADER%"
if %errorlevel% neq 0 (echo Error al generar el .h & pause & exit /b %errorlevel%)

echo [3/4] bin2s: Generando ensamblador .s...
:: La redirección > funciona nativamente en archivos .bat envolviendo el comando completo
"%TOOLS_SRC%\bin2s.exe" "%SHADER_SHBIN%" > "%SHADER_S%"
if %errorlevel% neq 0 (echo Error al generar el .s & pause & exit /b %errorlevel%)

echo [4/4] arm-as: Compilando objeto shader .o...
"%ARM_COMP%.exe" -march=armv6k -mfloat-abi=hard "%SHADER_S%" -o "%SHADER_OBJ%"
if %errorlevel% neq 0 (echo Error al compilar el objeto .o & pause & exit /b %errorlevel%)

echo ¡Proceso completado con éxito! El archivo .o está listo.
