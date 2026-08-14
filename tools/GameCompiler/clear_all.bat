@echo off
setlocal enabledelayedexpansion

for %%D in (
  build
  build_3ds
  romfs
) do (
    if exist "%%D" rmdir /s /q "%%D"
)

endlocal