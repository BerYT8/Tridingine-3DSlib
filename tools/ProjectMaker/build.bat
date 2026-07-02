@echo off
setlocal enabledelayedexpansion

REM ===== BUILD DIR =====
set BUILD_DIR=build

rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"

cd "%BUILD_DIR%"

cmake ..
cmake --build . --config Release

endlocal