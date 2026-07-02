@echo off
setlocal enabledelayedexpansion

set BUILD_DIR=build

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

cd "%BUILD_DIR%"
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

endlocal