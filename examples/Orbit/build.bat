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

    call :build3ds %2 %3 %4 %5 %6 %7 %8 %9

    goto :end
)

goto :next


:build3ds
set "CONTENT_DIR=content" 
set "ROMFS_DIR_3DS=romfs/3ds"

rem FontsConverter 
.\tools\FontsConverter.exe --all -3ds -i "%CONTENT_DIR%" -o "%ROMFS_DIR_3DS%" 

if errorlevel 1 goto :error

rem 3DModelsConverter 
.\tools\3DModelsConverter.exe --all -i "%CONTENT_DIR%" -o "%ROMFS_DIR_3DS%" 

if errorlevel 1 goto :error

rem SoundMaker3DS 
.\tools\SoundMaker3DS.exe --all -i "%CONTENT_DIR%" -o "%ROMFS_DIR_3DS%" 

if errorlevel 1 goto :error

rem LocalizationMaker 
.\tools\LocalizationMaker.exe --all -i "%CONTENT_DIR%" -o "%ROMFS_DIR_3DS%"

if errorlevel 1 goto :error

mkdir "%ROMFS_DIR_3DS%\certs"
copy "%CONTENT_DIR%\certs\ca-bundle.crt" "%ROMFS_DIR_3DS%\certs\ca-bundle.crt"

if errorlevel 1 goto :error

call "C:\devkitPro\msys2\msys2_shell.bat" -here -defterm -no-start -msys -c "./tools/build_3ds.sh %*"

goto :end

:next

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

    rem --------------------------------------------------------
    rem TITLE
    rem --------------------------------------------------------

    for /f "usebackq delims=" %%A in (`powershell -NoProfile -Command "(Get-Content -Raw 'game.json' | ConvertFrom-Json).title"`) do (
        set "GAME_TITLE=%%A"
    )

    if defined GAME_TITLE (
        echo Titulo detectado en game.json: "!GAME_TITLE!"
    )

    rem --------------------------------------------------------
    rem FILE
    rem --------------------------------------------------------

    for /f "usebackq delims=" %%A in (`powershell -NoProfile -Command "(Get-Content -Raw 'game.json' | ConvertFrom-Json).file"`) do (
        set "GAME_NAME=%%A"
    )

    if defined GAME_NAME (
        rem Reemplazar espacios por guiones bajos
        set "GAME_NAME=!GAME_NAME: =_!"

        echo Nombre de archivo detectado en game.json: "!GAME_NAME!"
    )

    rem --------------------------------------------------------
    rem AUTHOR
    rem --------------------------------------------------------

    for /f "usebackq delims=" %%A in (`powershell -NoProfile -Command "(Get-Content -Raw 'game.json' | ConvertFrom-Json).author"`) do (
        set "GAME_AUTHOR=%%A"
    )

    if defined GAME_AUTHOR (
        echo Autor detectado en game.json: "!GAME_AUTHOR!"
    )

    rem --------------------------------------------------------
    rem DESCRIPTION
    rem --------------------------------------------------------

    for /f "usebackq delims=" %%A in (`powershell -NoProfile -Command "(Get-Content -Raw 'game.json' | ConvertFrom-Json).description"`) do (
        set "GAME_DESC=%%A"
    )

    if defined GAME_DESC (
        echo Descripcion detectada en game.json: "!GAME_DESC!"
    )

    rem --------------------------------------------------------
    rem SOURCES
    rem --------------------------------------------------------

    for /f "usebackq delims=" %%A in (`powershell -NoProfile -Command "(Get-Content -Raw 'game.json' | ConvertFrom-Json).sources -join ';'"`) do (
        set "GAME_SOURCES=%%A"
    )

    if defined GAME_SOURCES (
        echo Sources detectados en game.json:
        echo   !GAME_SOURCES!
    )

    rem --------------------------------------------------------
    rem INCLUDES
    rem --------------------------------------------------------

    for /f "usebackq delims=" %%A in (`powershell -NoProfile -Command "(Get-Content -Raw 'game.json' | ConvertFrom-Json).includes -join ';'"`) do (
        set "GAME_INCLUDES=%%A"
    )

    if defined GAME_INCLUDES (
        echo Includes detectados en game.json:
        echo   !GAME_INCLUDES!
    )
)


:: ============================================================
:: CMAKE
:: ============================================================

cd "%BUILD_DIR%" || goto :error

:: ------------------------------------------------------------
:: Construir flags dinámicamente
:: ------------------------------------------------------------

set "CMAKE_FLAGS="

if defined GAME_TITLE (
    set CMAKE_FLAGS=!CMAKE_FLAGS! -DGAME_TITLE="!GAME_TITLE!"
)

if defined GAME_NAME (
    set CMAKE_FLAGS=!CMAKE_FLAGS! -DGAME_NAME="!GAME_NAME!"
)

if defined GAME_AUTHOR (
    set CMAKE_FLAGS=!CMAKE_FLAGS! -DGAME_AUTHOR="!GAME_AUTHOR!"
)

if defined GAME_DESC (
    set CMAKE_FLAGS=!CMAKE_FLAGS! -DGAME_DESC="!GAME_DESC!"
)

if defined GAME_SOURCES (
    set CMAKE_FLAGS=!CMAKE_FLAGS! -DGAME_SOURCES="!GAME_SOURCES!"
)

if defined GAME_INCLUDES (
    set CMAKE_FLAGS=!CMAKE_FLAGS! -DGAME_INCLUDES="!GAME_INCLUDES!"
)

echo Flags CMake:
echo !CMAKE_FLAGS!
echo.

cmake .. !CMAKE_FLAGS!

if errorlevel 1 goto :error

cmake --build . --config Release

if errorlevel 1 goto :error

:: Copiar DLLs
copy "..\lib\pc\*.dll" "Release\" >nul 2>&1

cd ..

copy build\compile_commands.json compile_commands.json

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

mkdir "%ROMFS_DIR_PC%\certs"
copy "%CONTENT_DIR%\certs\ca-bundle.crt" "%ROMFS_DIR_PC%\certs\ca-bundle.crt"

if errorlevel 1 goto :error

:: ============================================================
:: CREAR GAME.PAK
:: ============================================================

echo Creando game.pak...

"tools\PakMaker.exe" -c "%ROMFS_DIR_PC%" -o "%BUILD_DIR%\Release\game.pak"

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