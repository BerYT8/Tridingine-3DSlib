@echo off
setlocal enabledelayedexpansion

REM ===== BUILD DIR =====
set "BUILD_DIR=build"

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

REM ===== MODE =====
if "%1"=="3ds" (
    set BUILD_3DS=ON
) else (
    set BUILD_3DS=OFF
)

REM ===== PC BUILD =====
if "%BUILD_3DS%"=="OFF" (
    cd "%BUILD_DIR%"
    rmdir /s /q Lib
    mkdir Lib
    mkdir Code
    cd Code
    cmake ../.. -DCMAKE_BUILD_TYPE=Release -DBUILD_3DS=OFF
    cmake --build . --config Release

    cd ../..
    .\build.bat 3ds

    cd "%BUILD_DIR%"/Lib
    mkdir tools
    mkdir content
    mkdir content\game
    mkdir content\engine
    mkdir romfs
    mkdir romfs\pc
    mkdir romfs\3ds
    mkdir templates
    mkdir examples
    mkdir include
    mkdir lib
    cd lib
    mkdir pc
    mkdir 3ds
    cd ../..

    xcopy "..\tools\GameCompiler" "Lib" /E /I /Y
    copy "Code\Release\3ds_libs.lib" "Lib\lib\pc\3ds_libs.lib"
    copy "..\lib\x64\*.dll" "Lib\lib\pc\"
    copy "..\lib\x64\*.lib" "Lib\lib\pc\"
    copy "..\lib\glew\x64\glew32s.lib" "Lib\lib\pc\glew32s.lib"
    xcopy "..\include" "Lib\include" /E /I /Y
    copy "..\build_3ds\lib3ds_libs.a" "Lib\lib\3ds\3ds_libs.a"
    xcopy "..\examples" "Lib\examples" /E /I /Y
    xcopy "..\templates" "Lib\templates" /E /I /Y
    xcopy "..\content" "Lib\content" /E /I /Y

REM ===== PAK MAKER =====
    cd ..\tools\PakMaker

    .\build.bat

    cd ..\..\build

    copy "..\tools\PakMaker\build\Release\PakMaker.exe" "Lib\tools\PakMaker.exe"
REM ===== 3D MODELS CONVERTER =====
    cd ..\tools\3DModelsConverter

    .\build.bat

    cd ..\..\build

    copy "..\tools\3DModelsConverter\build\Release\3DModelsConverter.exe" "Lib\tools\3DModelsConverter.exe"
    
REM ===== SOUND MAKER 3DS =====
    cd ..\tools\SoundMaker3DS

    .\build.bat

    cd ..\..\build

    copy "..\tools\SoundMaker3DS\build\Release\SoundMaker3DS.exe" "Lib\tools\SoundMaker3DS.exe"
    
REM ===== LOCALIZATION MAKER =====
    cd ..\tools\LocalizationMaker

    .\build.bat

    cd ..\..\build

    copy "..\tools\LocalizationMaker\build\Release\LocalizationMaker.exe" "Lib\tools\LocalizationMaker.exe"

REM ===== PROJECT MAKER =====
    cd ..\tools\ProjectMaker

    "..\..\build\Lib\tools\PakMaker.exe" -c "..\..\build\Lib" -o "pak.pak"

    .\build.bat

    cd ..\..\build

    copy "..\tools\ProjectMaker\build\Release\ProjectMaker.exe" "ProjectMaker.exe"
    
    goto :end
)

REM ===== 3DS BUILD =====
echo Building for 3DS using MSYS2...

set "BUILD_DIR=build_3ds"

REM ===== PARSEAR JSON O VALORES POR DEFECTO =====
set "JSON_FILE=%~2"

rem Inicializar valores por defecto de respaldo
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
    
    rem Extraccion segura de valores respetando espacios usando PowerShell integrado
    for /f "usebackq delims=" %%I in (`powershell -Command "(Get-Content '%JSON_FILE%' | ConvertFrom-Json).file"`) do set "VAL_GAME3DS=%%I"
    for /f "usebackq delims=" %%I in (`powershell -Command "(Get-Content '%JSON_FILE%' | ConvertFrom-Json).title"`) do set "VAL_TITLE=%%I"
    for /f "usebackq delims=" %%I in (`powershell -Command "(Get-Content '%JSON_FILE%' | ConvertFrom-Json).author"`) do set "VAL_AUTHOR=%%I"
    for /f "usebackq delims=" %%I in (`powershell -Command "(Get-Content '%JSON_FILE%' | ConvertFrom-Json).description"`) do set "VAL_DESC=%%I"
    
    rem CORRECCIÓN: Eliminamos la linea que sobreescribia VAL_GAME3DS con VAL_TITLE.
    rem Ahora procesamos correctamente los espacios del campo 'file' asignado a VAL_GAME3DS.
    if not "!VAL_GAME3DS!"=="" (
        set "VAL_GAME3DS=!VAL_GAME3DS: =_!"
    )
)

call ".\3ds_shader_compiler.bat"

REM ===== EJECUTAR MSYS2 =====
:: 1. Ejecuta MSYS2, corre el .sh y espera a que termine completamente
call "C:\devkitPro\msys2\msys2_shell.bat" -here -defterm -no-start -msys -c "./build3ds.sh"

echo Direction is: "%BUILD_DIR%"

copy "%BUILD_DIR%\code\code.elf" "%BUILD_DIR%\code.elf"
copy "%BUILD_DIR%\code\lib3ds_libs.a" "%BUILD_DIR%\lib3ds_libs.a"

:end
endlocal
