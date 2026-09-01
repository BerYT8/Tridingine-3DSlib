@echo off
setlocal enabledelayedexpansion

git submodule sync --recursive
git submodule update --init --recursive

endlocal