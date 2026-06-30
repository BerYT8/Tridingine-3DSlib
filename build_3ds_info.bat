@echo off
setlocal enabledelayedexpansion

REM ===== CAPTURA DE PARÁMETROS O VALORES POR DEFECTO =====
set "PARAM_ELF=%~1"
if "!PARAM_ELF!"=="" set "PARAM_ELF=../../code.elf"

set "PARAM_GAME3DS=%~2"
if "!PARAM_GAME3DS!"=="" (
    set "PARAM_GAME3DS=Game3DS"
) else (
    rem Remplaza todos los espacios por guiones bajos (_)
    set "PARAM_GAME3DS=!PARAM_GAME3DS: =_!"
)

set "PARAM_TITLE=%~3"
if "!PARAM_TITLE!"=="" set "PARAM_TITLE=Juego 3DS"

set "PARAM_DESC=%~4"
if "!PARAM_DESC!"=="" set "PARAM_DESC=Juego hecho con 3DSLIB."

set "PARAM_AUTHOR=%~5"
if "!PARAM_AUTHOR!"=="" set "PARAM_AUTHOR=Tú"


REM ===== BUILD DIR =====
set "BUILD_DIR=build_3ds"
rem Definir SIN comillas internas para evitar duplicaciones
set "BUILD_DIR2=!PARAM_GAME3DS!"
set "BUILD_DIR3=code"

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

cd "%BUILD_DIR%"

if not exist "%BUILD_DIR2%" (
    mkdir "%BUILD_DIR2%"
)

cd "%BUILD_DIR2%"

if not exist "%BUILD_DIR3%" (
    mkdir "%BUILD_DIR3%"
)

cd ../..

REM ===== 3DS BUILD =====

echo Building for 3DS using MSYS2...
echo Using Game3DS name: !PARAM_GAME3DS!

rem CORRECCIÓN: Se limpian las comillas para el comando de MSYS2
call "C:\devkitPro\msys2\msys2_shell.bat" -here -defterm -no-start -msys -c "./build_3ds_info.sh ""build_3ds/!PARAM_GAME3DS!/!BUILD_DIR3!"" ""!PARAM_ELF!"" ""!PARAM_GAME3DS!"" ""!PARAM_TITLE!"" ""!PARAM_DESC!"" ""!PARAM_AUTHOR!"""

rem CORRECCIÓN: Windows usa barras invertidas (\) para el comando copy externo
copy "%BUILD_DIR%\%BUILD_DIR2%\%BUILD_DIR3%\%BUILD_DIR2%.3dsx" "%BUILD_DIR%\%BUILD_DIR2%\%BUILD_DIR2%.3dsx"

:end

endlocal
