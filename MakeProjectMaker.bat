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
if exist "%BUILD_DIR%\Code\Release\Tridingine.dll" (
    copy /y "%BUILD_DIR%\Code\Release\Tridingine.dll" "%LIB_DIR%\lib\pc\libTridingine.dll" >nul
) else (
    echo WARNING: Tridingine.dll not found.
)

REM Import/static library de la DLL
if exist "%BUILD_DIR%\Code\Release\TridingineEntrypoint.lib" (
    copy /y "%BUILD_DIR%\Code\Release\TridingineEntrypoint.lib" "%LIB_DIR%\lib\pc\libTridingineEntrypoint.lib" >nul
) else (
    echo WARNING: TridingineEntrypoint.lib not found.
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

set "TOOLS=PakMaker 3DModelsConverter LocalizationMaker FontsConverter"

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

        if exist "%ROOT%\tools\%%t\build\Release\%%t.exe" (
            copy /y "%ROOT%\tools\%%t\build\Release\%%t.exe" "%LIB_DIR%\tools\" >nul
        ) else if exist "%ROOT%\tools\%%t\build\%%t.exe" (
            copy /y "%ROOT%\tools\%%t\build\%%t.exe" "%LIB_DIR%\tools\" >nul
        )
    )
)

REM =========================
REM SoundMaker3DS
REM =========================
echo Building SoundMaker3DS...

set "SOUNDMAKER_DIR=%ROOT%\tools\SoundMaker3DS"
set "OPUS_DIR=%SOUNDMAKER_DIR%\libopus"
set "OPUSENC_DIR=%SOUNDMAKER_DIR%\libopusenc"
set "SOUNDMAKER_BUILD=%SOUNDMAKER_DIR%\build"
set "SOUNDMAKER_LIB=%SOUNDMAKER_DIR%\lib"

if not exist "%SOUNDMAKER_DIR%" (
    echo ERROR: SoundMaker3DS directory not found.
    exit /b 1
)

if not exist "%SOUNDMAKER_LIB%" (
    mkdir "%SOUNDMAKER_LIB%"
)

REM =========================
REM libopus
REM =========================
echo Building libopus...

cmake -S "%OPUS_DIR%" ^
      -B "%OPUS_DIR%\build" ^
      -DOPUS_BUILD_PROGRAMS=ON ^
      -DOPUS_BUILD_TESTING=ON ^
      -DCMAKE_BUILD_TYPE=Release

if !errorlevel! neq 0 (
    echo ERROR: libopus configure failed.
    exit /b !errorlevel!
)

cmake --build "%OPUS_DIR%\build" --config Release

if !errorlevel! neq 0 (
    echo ERROR: libopus build failed.
    exit /b !errorlevel!
)

REM =========================
REM Copiar opus.lib
REM =========================
echo Copying opus.lib...

if exist "%OPUS_DIR%\build\Release\opus.lib" (
    copy /y "%OPUS_DIR%\build\Release\opus.lib" ^
        "%SOUNDMAKER_LIB%\opus.lib" >nul
) else if exist "%OPUS_DIR%\build\opus.lib" (
    copy /y "%OPUS_DIR%\build\opus.lib" ^
        "%SOUNDMAKER_LIB%\opus.lib" >nul
) else if exist "%OPUS_DIR%\build\Release\libopus.lib" (
    copy /y "%OPUS_DIR%\build\Release\libopus.lib" ^
        "%SOUNDMAKER_LIB%\opus.lib" >nul
) else if exist "%OPUS_DIR%\build\libopus.lib" (
    copy /y "%OPUS_DIR%\build\libopus.lib" ^
        "%SOUNDMAKER_LIB%\opus.lib" >nul
) else (
    echo ERROR: opus.lib not found.
    exit /b 1
)

REM Comprobar copia
if not exist "%SOUNDMAKER_LIB%\opus.lib" (
    echo ERROR: Failed to copy opus.lib.
    exit /b 1
)

echo opus.lib copied successfully.

REM =========================
REM Preparar libopusenc
REM =========================
echo Preparing libopusenc...

if not exist "%SOUNDMAKER_DIR%\cmake\libopusenc.cmake" (
    echo ERROR: libopusenc.cmake not found.
    exit /b 1
)

copy /y ^
    "%SOUNDMAKER_DIR%\cmake\libopusenc.cmake" ^
    "%OPUSENC_DIR%\CMakeLists.txt" >nul

if !errorlevel! neq 0 (
    echo ERROR: Failed to prepare libopusenc.
    exit /b !errorlevel!
)

REM =========================
REM Copiar headers de Opus
REM =========================
echo Copying Opus headers...

if not exist "%OPUSENC_DIR%\lib_include" (
    mkdir "%OPUSENC_DIR%\lib_include"
)

copy /y ^
    "%OPUS_DIR%\include\*.h" ^
    "%OPUSENC_DIR%\lib_include\" >nul

if !errorlevel! neq 0 (
    echo ERROR: Failed to copy Opus headers.
    exit /b !errorlevel!
)

REM =========================
REM libopusenc
REM =========================
echo Building libopusenc...

cmake -S "%OPUSENC_DIR%" ^
      -B "%OPUSENC_DIR%\build" ^
      -DCMAKE_BUILD_TYPE=Release

if !errorlevel! neq 0 (
    echo ERROR: libopusenc configure failed.
    exit /b !errorlevel!
)

cmake --build "%OPUSENC_DIR%\build" --config Release

if !errorlevel! neq 0 (
    echo ERROR: libopusenc build failed.
    exit /b !errorlevel!
)

REM =========================
REM Copiar opusenc.lib
REM =========================
echo Copying opusenc.lib...

if exist "%OPUSENC_DIR%\build\Release\opusenc.lib" (
    copy /y ^
        "%OPUSENC_DIR%\build\Release\opusenc.lib" ^
        "%SOUNDMAKER_LIB%\opusenc.lib" >nul
) else if exist "%OPUSENC_DIR%\build\opusenc.lib" (
    copy /y ^
        "%OPUSENC_DIR%\build\opusenc.lib" ^
        "%SOUNDMAKER_LIB%\opusenc.lib" >nul
) else if exist "%OPUSENC_DIR%\build\Release\libopusenc.lib" (
    copy /y ^
        "%OPUSENC_DIR%\build\Release\libopusenc.lib" ^
        "%SOUNDMAKER_LIB%\opusenc.lib" >nul
) else if exist "%OPUSENC_DIR%\build\libopusenc.lib" (
    copy /y ^
        "%OPUSENC_DIR%\build\libopusenc.lib" ^
        "%SOUNDMAKER_LIB%\opusenc.lib" >nul
) else (
    echo ERROR: opusenc.lib not found.
    exit /b 1
)

if not exist "%SOUNDMAKER_LIB%\opusenc.lib" (
    echo ERROR: Failed to copy opusenc.lib.
    exit /b 1
)

echo opusenc.lib copied successfully.

REM =========================
REM SoundMaker3DS
REM =========================
echo Building SoundMaker3DS...

cmake -S "%SOUNDMAKER_DIR%" ^
      -B "%SOUNDMAKER_BUILD%" ^
      -DCMAKE_BUILD_TYPE=Release

if !errorlevel! neq 0 (
    echo ERROR: SoundMaker3DS configure failed.
    exit /b !errorlevel!
)

cmake --build "%SOUNDMAKER_BUILD%" --config Release

if !errorlevel! neq 0 (
    echo ERROR: SoundMaker3DS build failed.
    exit /b !errorlevel!
)

REM =========================
REM Copiar ejecutable
REM =========================
if exist "%SOUNDMAKER_BUILD%\Release\SoundMaker3DS.exe" (
    copy /y ^
        "%SOUNDMAKER_BUILD%\Release\SoundMaker3DS.exe" ^
        "%LIB_DIR%\tools\SoundMaker3DS.exe" >nul
) else if exist "%SOUNDMAKER_BUILD%\SoundMaker3DS.exe" (
    copy /y ^
        "%SOUNDMAKER_BUILD%\SoundMaker3DS.exe" ^
        "%LIB_DIR%\tools\SoundMaker3DS.exe" >nul
) else (
    echo ERROR: SoundMaker3DS.exe not found.
    exit /b 1
)

echo SoundMaker3DS built successfully.

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