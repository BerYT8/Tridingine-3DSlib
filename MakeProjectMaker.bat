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
mkdir "%LIB_DIR%\include"
mkdir "%LIB_DIR%\lib\pc"
mkdir "%LIB_DIR%\lib\3ds"

REM =========================
REM Assets
REM =========================
echo [2/6] Copying engine assets...

REM Asegurar que existan los origenes
if not exist "%ROOT%\include" mkdir "%ROOT%\include"
if not exist "%ROOT%\content" mkdir "%ROOT%\content"

REM Copiar GameCompiler
if exist "%ROOT%\tools\GameCompiler" (
    xcopy /e /i /y "%ROOT%\tools\GameCompiler\*" "%LIB_DIR%\" >nul 2>nul
)

REM Copiar include
xcopy /e /i /y "%ROOT%\include\*" "%LIB_DIR%\include\" >nul 2>nul

REM Copiar content
xcopy /e /i /y "%ROOT%\content\*" "%LIB_DIR%\content\" >nul 2>nul

REM =========================
REM PC libs
REM =========================
echo [3/6] Copying PC libs...

REM DLL principal
if exist "%BUILD_DIR%\Code\libTridingine.dll" (
    copy /y "%BUILD_DIR%\Code\libTridingine.dll" "%LIB_DIR%\lib\pc\libTridingine.dll" >nul
) else (
    echo WARNING: libTridingine.dll not found.
)

REM Import/static library de la DLL
if exist "%BUILD_DIR%\Code\libTridingineEntrypoint.lib" (
    copy /y "%BUILD_DIR%\Code\libTridingineEntrypoint.lib" "%LIB_DIR%\lib\pc\libTridingineEntrypoint.lib" >nul
) else (
    echo WARNING: libTridingineEntrypoint.lib not found.
)

REM =========================
REM 3DS libs
REM =========================
echo [4/6] Copying 3DS libs...

REM Mantener los nombres equivalentes a los .a de Linux
if exist "%ROOT%\build_3ds\libTridingine.a" (
    copy /y "%ROOT%\build_3ds\libTridingine.a" "%LIB_DIR%\lib\3ds\tridingine.a" >nul
) else (
    echo WARNING: build_3ds\libTridingine.a not found.
)

if exist "%ROOT%\build_3ds\libTridingineEntrypoint.a" (
    copy /y "%ROOT%\build_3ds\libTridingineEntrypoint.a" "%LIB_DIR%\lib\3ds\entrypoint.a" >nul
) else (
    echo WARNING: build_3ds\libTridingineEntrypoint.a not found.
)

REM =========================
REM Tools build
REM =========================
echo [5/6] Building tools...

set "TOOLS=PakMaker 3DModelsConverter SoundMaker3DS LocalizationMaker FontsConverter"

for %%t in (%TOOLS%) do (
    echo  -^> %%t

    if exist "%ROOT%\tools\%%t" (

        cmake -S "%ROOT%\tools\%%t" ^
              -B "%ROOT%\tools\%%t\build" ^
              -DCMAKE_BUILD_TYPE=Release

        if !errorlevel! neq 0 (
            echo ERROR: Failed to configure %%t
            exit /b !errorlevel!
        )

        cmake --build "%ROOT%\tools\%%t\build" --config Release

        if !errorlevel! neq 0 (
            echo ERROR: Failed to build %%t
            exit /b !errorlevel!
        )

        REM CMake en Windows suele colocar los ejecutables en Release
        if exist "%ROOT%\tools\%%t\build\Release\%%t.exe" (
            copy /y "%ROOT%\tools\%%t\build\Release\%%t.exe" "%LIB_DIR%\tools\" >nul
        ) else if exist "%ROOT%\tools\%%t\build\%%t.exe" (
            copy /y "%ROOT%\tools\%%t\build\%%t.exe" "%LIB_DIR%\tools\" >nul
        )
    )
)

REM =========================
REM bannertool
REM =========================
echo Building bannertool...

if exist "%ROOT%\tools\bannertool" (
    pushd "%ROOT%\tools\bannertool"

    make

    if !errorlevel! neq 0 (
        echo ERROR: bannertool build failed.
        popd
        exit /b !errorlevel!
    )

    popd

    REM Buscar binario Windows
    if exist "%ROOT%\tools\bannertool\output\windows-x86_64\bannertool.exe" (
        copy /y "%ROOT%\tools\bannertool\output\windows-x86_64\bannertool.exe" "%LIB_DIR%\tools\" >nul
    ) else if exist "%ROOT%\tools\bannertool\output\windows\bannertool.exe" (
        copy /y "%ROOT%\tools\bannertool\output\windows\bannertool.exe" "%LIB_DIR%\tools\" >nul
    ) else if exist "%ROOT%\tools\bannertool\bannertool.exe" (
        copy /y "%ROOT%\tools\bannertool\bannertool.exe" "%LIB_DIR%\tools\" >nul
    )
)

REM =========================
REM makerom
REM =========================
echo Building makerom...

if exist "%ROOT%\tools\Project_CTR\makerom" (
    pushd "%ROOT%\tools\Project_CTR\makerom"

    make deps
    if !errorlevel! neq 0 (
        echo ERROR: makerom dependencies failed.
        popd
        exit /b !errorlevel!
    )

    make
    if !errorlevel! neq 0 (
        echo ERROR: makerom build failed.
        popd
        exit /b !errorlevel!
    )

    popd

    if exist "%ROOT%\tools\Project_CTR\makerom\bin\makerom.exe" (
        copy /y "%ROOT%\tools\Project_CTR\makerom\bin\makerom.exe" "%LIB_DIR%\tools\" >nul
    )
)

REM =========================
REM 3dstool
REM =========================
echo Building 3dstool...

if exist "%ROOT%\tools\3dstool" (

    cmake -S "%ROOT%\tools\3dstool" ^
          -B "%ROOT%\tools\3dstool\build" ^
          -DCMAKE_BUILD_TYPE=Release

    if !errorlevel! neq 0 (
        echo ERROR: 3dstool configure failed.
        exit /b !errorlevel!
    )

    cmake --build "%ROOT%\tools\3dstool\build" --config Release

    if !errorlevel! neq 0 (
        echo ERROR: 3dstool build failed.
        exit /b !errorlevel!
    )

    if exist "%ROOT%\tools\3dstool\bin\Release\3dstool.exe" (
        copy /y "%ROOT%\tools\3dstool\bin\Release\3dstool.exe" "%LIB_DIR%\tools\" >nul
    ) else if exist "%ROOT%\tools\3dstool\bin\3dstool.exe" (
        copy /y "%ROOT%\tools\3dstool\bin\3dstool.exe" "%LIB_DIR%\tools\" >nul
    )
)

REM =========================
REM Generar PAK
REM =========================
echo Generating ProjectMaker PAK...

if not exist "%LIB_DIR%\tools\PakMaker.exe" (
    echo ERROR: PakMaker.exe not found.
    exit /b 1
)

"%LIB_DIR%\tools\PakMaker.exe" ^
    -c "%LIB_DIR%" ^
    -o "%ROOT%\tools\ProjectMaker\template.pak" ^
    -e "build" "build_3ds" "romfs" "examples"

if !errorlevel! neq 0 (
    echo ERROR: Failed to generate PAK.
    exit /b !errorlevel!
)

REM =========================
REM Crear directorio de build
REM =========================
if not exist "%ROOT%\tools\ProjectMaker\build" (
    mkdir "%ROOT%\tools\ProjectMaker\build"
)

REM =========================
REM ProjectMaker
REM =========================
echo [6/6] Building ProjectMaker...

cmake -S "%ROOT%\tools\ProjectMaker" ^
      -B "%ROOT%\tools\ProjectMaker\build" ^
      -DCMAKE_BUILD_TYPE=Release

if !errorlevel! neq 0 (
    echo ERROR: ProjectMaker configure failed.
    exit /b !errorlevel!
)

cmake --build "%ROOT%\tools\ProjectMaker\build" --config Release

if !errorlevel! neq 0 (
    echo ERROR: ProjectMaker build failed.
    exit /b !errorlevel!
)

REM Copiar ProjectMaker.exe
if exist "%ROOT%\tools\ProjectMaker\build\Release\ProjectMaker.exe" (
    copy /y "%ROOT%\tools\ProjectMaker\build\Release\ProjectMaker.exe" "%BUILD_DIR%\ProjectMaker.exe" >nul
) else if exist "%ROOT%\tools\ProjectMaker\build\ProjectMaker.exe" (
    copy /y "%ROOT%\tools\ProjectMaker\build\ProjectMaker.exe" "%BUILD_DIR%\ProjectMaker.exe" >nul
) else (
    echo ERROR: ProjectMaker.exe not found.
    exit /b 1
)

echo.
echo === DONE ===

endlocal