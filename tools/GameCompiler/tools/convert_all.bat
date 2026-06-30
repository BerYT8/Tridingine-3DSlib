@echo off
setlocal enabledelayedexpansion

if "%1"=="3ds" (
    set BUILD_3DS=ON
    set "OUTPUT_DIR=..\romfs\3ds"
) else (
    set BUILD_3DS=OFF
    set "OUTPUT_DIR=..\romfs\pc"
)

set "INPUT_DIR=..\content"
set "LANG_CONVERTOR=LocalizationMaker.exe"
set "EXT=langs"

for /r "%INPUT_DIR%" %%F in (*.%EXT%) do (

    set "FULLPATH=%%F"

    :: Obtener ruta relativa real (forma segura)
    set "RELATIVE=%%F"
    set "RELATIVE=!RELATIVE:%INPUT_DIR%=!"

    for %%A in ("!RELATIVE!") do set "RELDIR=%%~dpA"

    set "OUTDIR=!RELDIR!%OUTPUT_DIR%"

    if not exist "!OUTDIR!" (
        mkdir "!OUTDIR!"
    )

    echo Procesando: %%F

    "%LANG_CONVERTOR%" -i "%%F" -o "!OUTDIR!"
)

endlocal