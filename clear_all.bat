@echo off
setlocal enabledelayedexpansion

for %%D in (
    build
    build_3ds
    tools\3DModelsConverter\build
    tools\LocalizationMaker\build
    tools\PakMaker\build
    tools\ProjectMaker\build
    tools\SoundMaker3DS\build
) do (
    if exist "%%D" rmdir /s /q "%%D"
)

endlocal