@echo off
setlocal enabledelayedexpansion

if not exist ".\romfs" mkdir ".\romfs"
if not exist ".\romfs\3ds" mkdir ".\romfs\3ds"
if not exist ".\romfs\pc" mkdir ".\romfs\pc"
if not exist ".\content" mkdir ".\content"
if not exist ".\content\engine" mkdir ".\content\engine"
if not exist ".\content\game" mkdir ".\content\game"

set "BUILD_DIR=build"

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

if "%1"=="3ds" (
    set BUILD_3DS=ON
) else (
    set BUILD_3DS=OFF
)

if "%BUILD_3DS%"=="OFF" (
    cd "%BUILD_DIR%"

    cmake ..
    cmake --build . --config Release

    copy "..\lib\pc\*.dll" "Release\"
    
    cd ..

    "tools\PakMaker.exe" -c "romfs\pc" -o "%BUILD_DIR%\Release\game.pak"

    goto :end
)

echo Building for 3DS using MSYS2...

set "BUILD_DIR=build_3ds"

set "JSON_FILE=game.json"

set "VAL_FILE=../../code.elf"
set "VAL_GAME3DS=Game3DS"
set "VAL_TITLE=Juego 3DS"
set "VAL_DESC=Juego hecho con 3DSLIB."
set "VAL_AUTHOR=Tú"

if "%JSON_FILE%"=="" (
    echo No se especifico archivo JSON. Usando valores por defecto...
) else if not exist "%JSON_FILE%" (
    echo El archivo JSON "%JSON_FILE%" no existe. Usando valores por defecto...
) else (
    echo Leyendo datos desde %JSON_FILE%...
    
    for /f "usebackq delims=" %%I in (`powershell -Command "(Get-Content '%JSON_FILE%' | ConvertFrom-Json).file"`) do set "VAL_GAME3DS=%%I"
    for /f "usebackq delims=" %%I in (`powershell -Command "(Get-Content '%JSON_FILE%' | ConvertFrom-Json).title"`) do set "VAL_TITLE=%%I"
    for /f "usebackq delims=" %%I in (`powershell -Command "(Get-Content '%JSON_FILE%' | ConvertFrom-Json).author"`) do set "VAL_AUTHOR=%%I"
    for /f "usebackq delims=" %%I in (`powershell -Command "(Get-Content '%JSON_FILE%' | ConvertFrom-Json).description"`) do set "VAL_DESC=%%I"
    
    if not "!VAL_GAME3DS!"=="" (
        set "VAL_GAME3DS=!VAL_GAME3DS: =_!"
    )
)

cd tools

call "C:\devkitPro\msys2\msys2_shell.bat" -here -defterm -no-start -msys -c "./build3ds.sh"

echo Direction is: "%BUILD_DIR%"

cd ..

copy "%BUILD_DIR%\code\code.elf" "%BUILD_DIR%\code.elf"
copy "%BUILD_DIR%\code\lib3ds_libs.a" "%BUILD_DIR%\lib3ds_libs.a"

cd tools

echo.
echo File: !VAL_FILE!
echo.
echo Name: !VAL_GAME3DS!
echo.
echo Title: !VAL_TITLE!
echo.
echo Desc: !VAL_DESC!
echo.
echo Author: !VAL_AUTHOR!
echo.

echo Pasando parametros a build_3ds_info.bat...
call ".\build_3ds_info.bat" "!VAL_FILE!" "!VAL_GAME3DS!" "!VAL_TITLE!" "!VAL_DESC!" "!VAL_AUTHOR!"

:end
endlocal