@echo off
setlocal enabledelayedexpansion

set "PARAM_ELF=%~1"
if "!PARAM_ELF!"=="" set "PARAM_ELF=../../code.elf"

set "PARAM_GAME3DS=%~2"
if "!PARAM_GAME3DS!"=="" (
    set "PARAM_GAME3DS=Game3DS"
) else (
    set "PARAM_GAME3DS=!PARAM_GAME3DS: =_!"
)

set "PARAM_TITLE=%~3"
if "!PARAM_TITLE!"=="" set "PARAM_TITLE=Juego 3DS"

set "PARAM_DESC=%~4"
if "!PARAM_DESC!"=="" set "PARAM_DESC=Juego hecho con 3DSLIB."

set "PARAM_AUTHOR=%~5"
if "!PARAM_AUTHOR!"=="" set "PARAM_AUTHOR=Tú"


set "BUILD_DIR=..\build_3ds"
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

cd ../../tools

echo Building for 3DS using MSYS2...
echo Using Game3DS name: !PARAM_GAME3DS!

call "C:\devkitPro\msys2\msys2_shell.bat" -here -defterm -no-start -msys -c "./build_3ds_info.sh ""!BUILD_DIR!/!PARAM_GAME3DS!/!BUILD_DIR3!"" ""!PARAM_ELF!"" ""!PARAM_GAME3DS!"" ""!PARAM_TITLE!"" ""!PARAM_DESC!"" ""!PARAM_AUTHOR!"""

copy "%BUILD_DIR%\%BUILD_DIR2%\%BUILD_DIR3%\%BUILD_DIR2%.3dsx" "%BUILD_DIR%\%BUILD_DIR2%\%BUILD_DIR2%.3dsx"

:end

endlocal
