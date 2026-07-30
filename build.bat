@echo off
setlocal

set "BUILD_DIR=build"

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM =========================
REM 3DS BUILD (MSYS2)
REM =========================
echo Building 3DS...

set "MSYS2=%DEVKITPRO%\msys2\msys2_shell.bat"

if not exist "%MSYS2%" (
    echo ERROR: DEVKITPRO no encontrado
    exit /b 1
)

call "%MSYS2%" -here -defterm -no-start -msys -c "./build_3ds.sh build_3ds"

REM =========================
REM PC BUILD
REM =========================
cd "%BUILD_DIR%"
rmdir /s /q Lib 2>nul
mkdir Lib
mkdir Code

cd Code
cmake ../.. -DBUILD_3DS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

cd ..\..

echo Done.
endlocal