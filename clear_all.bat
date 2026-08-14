@echo off
setlocal enabledelayedexpansion

for %%D in (
  build
  build_3ds
  tools/3DModelsConverter/build
  tools/LocalizationMaker/build
  tools/PakMaker/build
  tools/ProjectMaker/build
  tools/SoundMaker3DS/build
  tools/3dstool/build
  tools/3dstool/bin
  tools/bannertool/build
  tools/bannertool/output
  tools/Project_CTR/ctrtool/build
  tools/Project_CTR/makerom/build
  tools/Project_CTR/makerom/bin
) do (
    if exist "%%D" rmdir /s /q "%%D"
)

endlocal