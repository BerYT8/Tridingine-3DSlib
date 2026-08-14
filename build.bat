@echo off
setlocal

set "BUILD_DIR=build"

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM =========================
REM PC BUILD
REM =========================
echo =========================
echo Building PC...
echo =========================

cd "%BUILD_DIR%"

REM Limpiar y recrear directorios
rmdir /s /q Lib 2>nul
mkdir Lib
mkdir Code

cd Code

cmake ../.. -DBUILD_3DS=OFF -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo ERROR: CMake configuration failed.
    cd ..\..
    endlocal
    exit /b 1
)

cmake --build . --config Release
if errorlevel 1 (
    echo ERROR: PC build failed.
    cd ..\..
    endlocal
    exit /b 1
)

cd ..\..

REM =========================
REM 3DS BUILD
REM =========================
echo.
echo =========================
echo Building 3DS...
echo =========================

REM Verificar DEVKITPRO
if not defined DEVKITPRO (
    echo WARNING: La variable DEVKITPRO no esta definida.
    echo El build podria fallar.
)

REM Verificar MSYS2
set "MSYS2_BASH=%DEVKITPRO%\msys2\msys2_shell.bat"

if not exist "%MSYS2_BASH%" (
    echo ERROR: No se encuentra msys2_shell.bat de MSYS2.
    echo Ruta esperada: %MSYS2_BASH%
    endlocal
    exit /b 1
)

REM Ejecutar build_3ds.sh directamente
"%MSYS2_BASH%" -defterm -here -no-start -c "chmod +x ./build_3ds.sh && ./build_3ds.sh build_3ds"

if errorlevel 1 (
    echo ERROR: 3DS build failed.
    endlocal
    exit /b 1
)

echo.
echo Done.

endlocal
