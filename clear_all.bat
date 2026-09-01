@echo off
setlocal enabledelayedexpansion

for %%D in (
  build
  build_3ds
  tools/FontsConverter/build
  tools/3DModelsConverter/build
  tools/LocalizationMaker/build
  tools/PakMaker/build
  tools/ProjectMaker/build
  tools/SoundMaker3DS/build
  tools/SoundMaker3DS/libopus/build
  tools/SoundMaker3DS/libopusenc/build
  tools/bannertool/build
  tools/bannertool/output
  tools/Project_CTR/ctrtool/build
  tools/Project_CTR/makerom/build
  tools/Project_CTR/makerom/bin
  tools/3dstool/bin
  tools/3dstool/build
) do (
    if exist "%%D" rmdir /s /q "%%D"
)

endlocal