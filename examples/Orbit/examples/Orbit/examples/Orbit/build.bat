@echo off
setlocal enabledelayedexpansion

set "CONTENT_DIR=content"
set "ROMFS_DIR=romfs\pc"

:: Crear directorios base si no existen
for %%D in (romfs romfs\3ds romfs\pc content content\engine content\game) do (
    if not exist ".\%%D" mkdir ".\%%D"
)

if "%1"=="3ds" (
    echo Detectado parametro 3DS. Iniciando compilacion con MSYS2...
    call "C:\devkitPro\msys2\msys2_shell.bat" -here -defterm -no-start -msys -c "./tools/build_3ds.sh"
    goto :end
)

:: Compilación por defecto (PC)
echo Compilando version para PC...
set "BUILD_DIR=build"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

cd "%BUILD_DIR%"
cmake ..
cmake --build . --config Release
copy "..\lib\pc\*.dll" "Release\"
cd ..

"tools\3DModelsConverter.exe" --all -i "%CONTENT_DIR%" -o "%ROMFS_DIR%"
"tools\SoundMaker3DS.exe" --all -i "%CONTENT_DIR%" -o "%ROMFS_DIR%"
"tools\LocalizationMaker.exe" --all -i "%CONTENT_DIR%" -o "%ROMFS_DIR%"

"tools\PakMaker.exe" -c "romfs\pc" -o "%BUILD_DIR%\Release\game.pak"

:end
endlocal
