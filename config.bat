@echo off
setlocal enabledelayedexpansion

REM =========================
REM Descargar certificado CA
REM =========================

set "CONTENT_DIR=content"

set "CA_URL=https://curl.se/ca/cacert.pem"
set "CA_FILE=%CONTENT_DIR%\certs\ca-bundle.crt"

if not exist "%CONTENT_DIR%\certs" (
    mkdir "%CONTENT_DIR%\certs"
)

curl -L --fail "%CA_URL%" -o "%CA_FILE%"

if errorlevel 1 (
    echo.
    echo ERROR: No se ha podido descargar el certificado CA.
    echo.
    exit /b 1
)

if not exist "%CA_FILE%" (
    echo.
    echo ERROR: El certificado CA no existe.
    echo.
    exit /b 1
)

for %%A in ("%CA_FILE%") do if %%~zA==0 (
    echo.
    echo ERROR: El certificado CA esta vacio.
    echo.
    exit /b 1
)

echo.
echo Certificado descargado correctamente:
echo %CA_FILE%

git submodule sync --recursive
git submodule update --init --recursive

set "MSYS2_BASH=%DEVKITPRO%\msys2\msys2_shell.bat"
call "%MSYS2_BASH%" -defterm -here -no-start -c "pacman -Syu 3ds-libogg 3ds-opusfile 3ds-mbedtls"

endlocal