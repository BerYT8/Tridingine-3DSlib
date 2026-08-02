@echo off
setlocal enabledelayedexpansion

set "ROOT=%cd%"
set "BUILD_DIR=%ROOT%\build"
set "LIB_DIR=%BUILD_DIR%\Lib"

echo === ProjectMaker POST BUILD ===

REM =========================
REM Crear estructura Lib
REM =========================
echo [1/6] Creating Lib structure...

if exist "%LIB_DIR%" rmdir /s /q "%LIB_DIR%"

mkdir "%LIB_DIR%"
mkdir "%LIB_DIR%\tools"
mkdir "%LIB_DIR%\content\game"
mkdir "%LIB_DIR%\content\engine"
mkdir "%LIB_DIR%\romfs\pc"
mkdir "%LIB_DIR%\romfs\3ds"
mkdir "%LIB_DIR%\templates"
mkdir "%LIB_DIR%\examples"
mkdir "%LIB_DIR%\include"
mkdir "%LIB_DIR%\lib\pc"
mkdir "%LIB_DIR%\lib\3ds"

REM =========================
REM Assets
REM =========================
echo [2/6] Copying engine assets...

if exist "%ROOT%\tools\GameCompiler" xcopy /e /i /y "%ROOT%\tools\GameCompiler" "%LIB_DIR%" >nul 2>nul
if exist "%ROOT%\include" xcopy /e /i /y "%ROOT%\include" "%LIB_DIR%\include" >nul 2>nul
if exist "%ROOT%\examples" xcopy /e /i /y "%ROOT%\examples" "%LIB_DIR%\examples" >nul 2>nul
if exist "%ROOT%\templates" xcopy /e /i /y "%ROOT%\templates" "%LIB_DIR%\templates" >nul 2>nul
if exist "%ROOT%\content" xcopy /e /i /y "%ROOT%\content" "%LIB_DIR%\content" >nul 2>nul

REM =========================
REM PC libs
REM =========================
echo [3/6] Copying PC libs...

copy /y "%BUILD_DIR%\Code\Release\3ds_libs.lib" "%LIB_DIR%\lib\pc\" >nul 2>nul
copy /y "%ROOT%\lib\x64\*.dll" "%LIB_DIR%\lib\pc\" >nul 2>nul
copy /y "%ROOT%\lib\x64\*.lib" "%LIB_DIR%\lib\pc\" >nul 2>nul
copy /y "%ROOT%\lib\glew\x64\glew32s.lib" "%LIB_DIR%\lib\pc\" >nul 2>nul

REM =========================
REM 3DS lib
REM =========================
echo [4/6] Copying 3DS libs...

copy /y "%ROOT%\build_3ds\lib3ds_libs.a" "%LIB_DIR%\lib\3ds\3ds_libs.a" >nul 2>nul

REM =========================
REM Tools build
REM =========================
echo [5/6] Building tools...

set "TOOLS=PakMaker 3DModelsConverter SoundMaker3DS LocalizationMaker FontsConverter"

for %%t in (%TOOLS%) do (
    echo  ^-^> %%t
    if exist "%ROOT%\tools\%%t" (
        cmake -S "%ROOT%\tools\%%t" -B "%ROOT%\tools\%%t\build" -DCMAKE_BUILD_TYPE=Release
        if !errorlevel! neq 0 exit /b !errorlevel!
        
        cmake --build "%ROOT%\tools\%%t\build" --config Release
        if !errorlevel! neq 0 exit /b !errorlevel!
        
        if exist "%ROOT%\tools\%%t\build\Release" (
            copy /y "%ROOT%\tools\%%t\build\Release\*.*" "%LIB_DIR%\tools\" >nul 2>nul
        ) else (
            copy /y "%ROOT%\tools\%%t\build\*.*" "%LIB_DIR%\tools\" >nul 2>nul
        )
    )
)

"%LIB_DIR%\tools\PakMaker.exe" -c "%LIB_DIR%" -o "%ROOT%\tools\ProjectMaker\pak.pak"

REM =========================
REM ProjectMaker
REM =========================
echo [6/6] Building ProjectMaker...

cmake -S "%ROOT%\tools\ProjectMaker" -B "%ROOT%\tools\ProjectMaker\build" -DCMAKE_BUILD_TYPE=Release
if !errorlevel! neq 0 exit /b !errorlevel!

cmake --build "%ROOT%\tools\ProjectMaker\build" --config Release
if !errorlevel! neq 0 exit /b !errorlevel!

if exist "%ROOT%\tools\ProjectMaker\build\Release\ProjectMaker.exe" (
    copy /y "%ROOT%\tools\ProjectMaker\build\Release\ProjectMaker.exe" "%BUILD_DIR%\ProjectMaker.exe" >nul 2>nul
) else (
    copy /y "%ROOT%\tools\ProjectMaker\build\ProjectMaker.exe" "%BUILD_DIR%\ProjectMaker.exe" >nul 2>nul
)

echo === DONE ===
endlocal
