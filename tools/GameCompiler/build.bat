@echo off
setlocal enabledelayedexpansion

:: ============================================================
:: CREAR DIRECTORIOS BASE
:: ============================================================

for %%D in (
    romfs
    romfs\3ds
    romfs\pc
    content
    content\engine
    content\game
) do (
    if not exist ".\%%D" mkdir ".\%%D"
)

:: ============================================================
:: DAR PERMISOS / COMPROBAR HERRAMIENTAS
:: ============================================================

set "CONTENT_DIR=content"
set "ROMFS_DIR_3DS=romfs\3ds"
set "ROMFS_DIR_PC=romfs\pc"

:: ============================================================
:: COMPILACION 3DS
:: ============================================================

if /I "%~1"=="3ds" (
    echo Iniciando compilacion para 3DS...

    call "C:\devkitPro\msys2\msys2_shell.bat" -here -defterm -no-start -msys -c "./tools/build_3ds.sh"

    goto :end
)

:: ============================================================
:: COMPILACION PC / NATIVO
:: ============================================================

echo Compilando version para PC...

set "BUILD_DIR=build"

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

:: ============================================================
:: LEER DATOS DESDE GAME.JSON
:: ============================================================

set "GAME_TITLE="
set "GAME_NAME="

if exist "game.json" (
    echo Detectado game.json. Leyendo informacion...

    :: Extraer title
    for /f "tokens=2 delims=:," %%A in ('findstr /C:"\"title\"" game.json') do (
        set "GAME_TITLE=%%~A"
        set "GAME_TITLE=!GAME_TITLE:"=!"
        set "GAME_TITLE=!GAME_TITLE: =!"
    )

    if defined GAME_TITLE (
        echo Titulo detectado en game.json: "!GAME_TITLE!"
    )

    :: Extraer file
    for /f "tokens=2 delims=:," %%A in ('findstr /C:"\"file\"" game.json') do (
        set "GAME_NAME=%%~A"
        set "GAME_NAME=!GAME_NAME:"=!"
        set "GAME_NAME=!GAME_NAME: =!"
    )

    if defined GAME_NAME (
        :: Reemplazar espacios por guiones bajos
        set "GAME_NAME=!GAME_NAME: =_!"

        echo Nombre de archivo detectado en game.json: "!GAME_NAME!"
    )
)

:: ============================================================
:: CMAKE
:: ============================================================

cd "%BUILD_DIR%" || goto :error

if defined GAME_TITLE (
    if defined GAME_NAME (
        cmake .. -DGAME_TITLE="!GAME_TITLE!" -DGAME_NAME="!GAME_NAME!"
    ) else (
        cmake .. -DGAME_TITLE="!GAME_TITLE!"
    )
) else (
    if defined GAME_NAME (
        cmake .. -DGAME_NAME="!GAME_NAME!"
    ) else (
        cmake ..
    )
)

if errorlevel 1 goto :error

cmake --build . --config Release

if errorlevel 1 goto :error

:: Copiar DLLs
copy "..\lib\pc\*.dll" "Release\" >nul 2>&1

cd ..

:: ============================================================
:: PROCESAR CONTENT PARA PC
:: ============================================================

echo Procesando contenido para PC...

"tools\FontsConverter.exe" --all -pc -i "%CONTENT_DIR%" -o "%ROMFS_DIR_PC%"

if errorlevel 1 goto :error

"tools\3DModelsConverter.exe" --all -i "%CONTENT_DIR%" -o "%ROMFS_DIR_PC%"

if errorlevel 1 goto :error

"tools\SoundMaker3DS.exe" --all -i "%CONTENT_DIR%" -o "%ROMFS_DIR_PC%"

if errorlevel 1 goto :error

"tools\LocalizationMaker.exe" --all -i "%CONTENT_DIR%" -o "%ROMFS_DIR_PC%"

if errorlevel 1 goto :error

:: ============================================================
:: CREAR GAME.PAK
:: ============================================================

echo Creando game.pak...

"tools\PakMaker.exe" -c "%ROMFS_DIR_PC%" -o "%BUILD_DIR%\game.pak"

if errorlevel 1 goto :error

echo.
echo ========================================
echo Compilacion completada correctamente.
echo ========================================
goto :end

:: ============================================================
:: ERROR
:: ============================================================

:error
echo.
echo ========================================
echo ERROR: La compilacion ha fallado.
echo ========================================
exit /b 1

:: ============================================================
:: FIN
:: ============================================================

:end
endlocal