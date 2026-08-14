# Tridingine — Dependencies & Setup

This document explains the dependencies required to build Tridingine for PC and Nintendo 3DS.

## Table of Contents

- [General Requirements](#general-requirements)
- [Nintendo 3DS — devkitPro](#nintendo-3ds--devkitpro)
  - [Windows](#windows)
  - [Linux](#linux)
- [PC — Windows](#pc--windows)
  - [Microsoft Visual Studio](#microsoft-visual-studio)
  - [GLEW](#glew)
  - [CMAKE_PREFIX_PATH](#cmake_prefix_path)
- [PC — Linux](#pc--linux)
  - [GLEW](#glew-1)
- [Building Tridingine](#building-tridingine)
- [Building a Game](#building-a-game)
- [Troubleshooting](#troubleshooting)


# General Requirements

Tridingine uses CMake as its build system.

You need:

- CMake 3.10 or newer
- A C compiler
- A C++ compiler with C++20 support
- Git, if cloning the repository
- The required platform-specific dependencies

Check your CMake installation with:

```bash
cmake --version
```

The project uses C++20.


# Nintendo 3DS — devkitPro

Nintendo 3DS builds require devkitPro and the following 3DS development packages:

- devkitARM
- libctru
- citro2d
- citro3d
- libogg
- libopus
- libopusfile
- Other packages required by the devkitPro 3DS environment

The official devkitPro setup instructions are available here:

https://devkitpro.org/wiki/Getting_Started

devkitPro is the primary requirement for building Tridingine for Nintendo 3DS.


## Windows

On Windows, the recommended way to install devkitPro is using the official devkitPro installer.

Follow the official instructions:

https://devkitpro.org/wiki/Getting_Started

During installation, make sure the Nintendo 3DS development environment is installed, including the required portlibs.

After installation, verify that the required environment is available.

For example, check:

```powershell
echo %DEVKITPRO%
```

You should have a path similar to:

```text
C:\devkitPro
```

The exact path may be different on your system.

## Windows

On Windows, the recommended way to install devkitPro is using the official devkitPro installer.

Follow the official instructions:

https://devkitpro.org/wiki/Getting_Started

During installation, make sure the Nintendo 3DS development environment is installed, including the required 3DS portlibs.

### Important Windows Configuration

After installing devkitPro, some environment variables may be configured incorrectly for CMake.

You should manually check the Windows environment variables before building.

Open:

**Windows Settings → System → About → Advanced system settings → Environment Variables**

or search for:

**"Edit the system environment variables"**

### Remove `C:\devkitPro\msys\bin` from `Path`

In **System variables**, find:

```text
Path
```

Edit it and remove the entry:

```text
C:\devkitPro\msys\bin
```

This entry can cause conflicts with the development tools used by CMake and may result in errors during configuration or compilation.

Do not remove the entire `Path` variable. Only remove the `C:\devkitPro\msys\bin` entry.

### Fix `DEVKITPRO`, `DEVKITARM` and `DEVKITPPC`

The devkitPro installer may create these variables with Linux-style paths such as:

```text
/opt/devkitpro
/opt/devkitpro/devkitARM
/opt/devkitpro/devkitPPC
```

These paths are incorrect for a native Windows build.

Change them to Windows paths.

Set:

```text
DEVKITPRO=C:\devkitPro
```

```text
DEVKITARM=C:\devkitPro\devkitARM
```

```text
DEVKITPPC=C:\devkitPro\devkitPPC
```

Alternatively, forward slashes can be used:

```text
DEVKITPRO=C:/devkitPro
```

```text
DEVKITARM=C:/devkitPro/devkitARM
```

```text
DEVKITPPC=C:/devkitPro/devkitPPC
```

The exact capitalization of `devkitPro` is not important on Windows, but the paths must point to the actual installation directory.

### Restart the Terminal

After changing the environment variables, close all open terminals.

Open a new terminal so that the updated environment is loaded.

You can verify the variables with:

```powershell
echo $env:DEVKITPRO
echo $env:DEVKITARM
echo $env:DEVKITPPC
```

Or from `cmd.exe`:

```cmd
echo %DEVKITPRO%
echo %DEVKITARM%
echo %DEVKITPPC%
```

They should point to paths similar to:

```text
C:\devkitPro
C:\devkitPro\devkitARM
C:\devkitPro\devkitPPC
```

Do not leave them pointing to:

```text
/opt/devkitpro
/opt/devkitpro/devkitARM
/opt/devkitpro/devkitPPC
```

Those are Linux-style paths and will cause files and tools to be incorrectly located when building the Nintendo 3DS version on Windows.

### Verify the Installation

Before configuring Tridingine, verify that devkitPro is correctly detected.

For example:

```cmd
echo %DEVKITPRO%
echo %DEVKITARM%
```

You should see something similar to:

```text
C:\devkitPro
C:\devkitPro\devkitARM
```

Once these variables are correctly configured, open a new terminal and configure the Tridingine 3DS build:

```cmd
cmake .. -DBUILD_3DS=ON
```

Then build:

```cmd
cmake --build .
```

If CMake still reports paths beginning with `/opt/devkitpro` on Windows, check the environment variables again and make sure you are using a newly opened terminal.


## Linux

On Linux, follow the official devkitPro instructions:

https://devkitpro.org/wiki/Getting_Started

The devkitPro wiki provides the current installation commands and package names.

After installation, make sure the following environment variable is available:

```bash
echo $DEVKITPRO
```

For example:

```text
/opt/devkitpro
```

The exact installation directory may be different depending on your system.


# PC — Windows

To build Tridingine for Windows, the following are required:

- CMake
- A Microsoft C/C++ compiler
- OpenGL
- GLEW
- SDL2
- SDL2_image
- SDL2_mixer
- SDL2_ttf

SDL2 and its related libraries are included in the Tridingine source tree under:

```text
external/
```

Therefore, you normally do not need to install SDL2 separately.

The project builds the required SDL2 libraries as part of the CMake configuration.


## Microsoft Visual Studio

Using Microsoft Visual Studio is highly recommended on Windows.

For the best experience, install:

**Visual Studio Community 2026**

or another version of Visual Studio that provides the Microsoft MSVC C/C++ compiler.

The important requirement is that the MSVC compiler and Windows development tools are installed.

Make sure the C++ development workload is installed.

The exact Visual Studio version is not mandatory as long as a compatible MSVC compiler is available.


## GLEW

Tridingine requires GLEW for the PC OpenGL build.

Windows users can download GLEW from the official releases:

https://github.com/nigels-com/glew/releases

For example, download:

```text
glew-2.3.1-win32
```

Extract it to:

```text
C:\glew-2.3.1
```

The resulting directory should contain the GLEW installation files.


## CMAKE_PREFIX_PATH

CMake needs to know where GLEW is installed.

On Windows, create the following environment variable:

```text
CMAKE_PREFIX_PATH
```

Set its value to:

```text
C:\glew-2.3.1
```

If you already have other CMake package paths in this variable, separate them with:

```text
;
```

For example:

```text
C:\glew-2.3.1;C:\other\cmake\packages
```

After creating or modifying `CMAKE_PREFIX_PATH`, **close and reopen your terminal**.

This is important because existing terminal sessions may not see newly created environment variables.

You can verify the variable with:

```powershell
echo %CMAKE_PREFIX_PATH%
```

Then CMake should be able to find GLEW using:

```cmake
find_package(GLEW REQUIRED)
```


# PC — Linux

Linux users also need:

- CMake
- GCC or Clang
- OpenGL development libraries
- GLEW
- The required SDL2 development dependencies

The exact installation commands depend on your Linux distribution.

For Debian/Ubuntu-based distributions, GLEW can generally be installed using the distribution package manager.

For example:

```bash
sudo apt update
sudo apt install libglew-dev
```

You may also need the OpenGL development packages required by your distribution.

For example:

```bash
sudo apt install libgl1-mesa-dev libglu1-mesa-dev
```

SDL2 dependencies are handled by the project where applicable, but the system may still require additional development packages depending on your configuration.


# OpenGL

The PC version requires OpenGL.

CMake searches for OpenGL using:

```cmake
find_package(OpenGL REQUIRED)
```

The required OpenGL development files must therefore be available on the system.

On Linux, install the OpenGL development packages provided by your distribution.

On Windows, the required OpenGL system libraries are normally provided by Windows and the graphics driver.


# Opus, Opusfile and Ogg

The Nintendo 3DS version uses the following audio libraries:

- libogg
- libopus
- libopusfile

These libraries are distributed as devkitPro 3DS portlibs, but they may **not be installed by default** with the basic devkitPro installation.

They must be installed separately before building Tridingine for Nintendo 3DS.

## Linux

On Linux, install the required 3DS portlibs using pacman:

```bash
sudo pacman -S 3ds-libogg 3ds-opus 3ds-opusfile
```

Depending on the devkitPro package repository and installed environment, additional dependencies may be installed automatically.

You can search for available 3DS packages with:

```bash
pacman -Ss 3ds-
```

The important packages for Tridingine's audio system are:

```text
3ds-libogg
3ds-opus
3ds-opusfile
```

After installing them, the libraries should be available to the devkitARM linker.

Tridingine links them using:

```cmake
target_link_libraries(
    TridingineCore
    PUBLIC
        opusfile
        opus
        ogg
)
```

## Windows

When using the devkitPro Windows environment, make sure the corresponding 3DS portlibs are installed through the devkitPro package manager/environment.

The required packages are:

```text
3ds-libogg
3ds-opus
3ds-opusfile
```

If these packages are missing, the final linker may report errors such as:

```text
cannot find -lopusfile
cannot find -lopus
cannot find -logg
```

Install the missing 3DS portlibs through the devkitPro environment and make sure that `DEVKITPRO` and `DEVKITARM` point to the correct Windows installation paths.

## Important

Do **not** copy the Ogg, Opus or Opusfile headers or libraries manually into the Tridingine source tree.

They should be installed as 3DS portlibs and provided to the compiler and linker through the devkitPro environment.

If the linker reports:

```text
cannot find -lopusfile
cannot find -lopus
cannot find -logg
```

this normally means that the corresponding 3DS portlibs are not installed or that the devkitPro environment is not configured correctly.


# Building Tridingine

## PC

### Linux

Use:
```bash
chmod +x config.sh
./congif.sh

./build.sh
./MakeProjectMaker.sh
```

### Windows

Use:
```bash
.\congif.bat

.\build.bat
.\MakeProjectMaker.bat
```

# Building a Game

# PC Game Build

For a PC game:

```bash
./build.sh
```

# Nintendo 3DS Game Build

For a Nintendo 3DS game:

```bash
./build.sh 3ds (link) (-a [ip]192.168.0.0)
```

You can also use **link** option with optional **-a** for direct connection. 
>You need your 3ds with 3dslink connection opened on homebrew with Y.

The 3DS build requires devkitPro and devkitARM to be correctly installed and configured.

## Cannot find opusfile, opus or ogg

If the linker reports:

```text
cannot find -lopusfile
cannot find -lopus
cannot find -logg
```

verify that:

- devkitPro is installed
- devkitARM is installed
- The Nintendo 3DS development environment is installed
- The 3DS portlibs are installed
- `DEVKITPRO` is correctly configured
- You are building with the devkitPro 3DS toolchain


## GLEW cannot be found on Windows

Verify that GLEW is installed, for example:

```text
C:\glew-2.3.1
```

Then verify:

```powershell
echo %CMAKE_PREFIX_PATH%
```

It should contain:

```text
C:\glew-2.3.1
```

Close and reopen the terminal after changing the environment variable.


# Official Resources

## devkitPro

Getting Started:

https://devkitpro.org/wiki/Getting_Started

## GLEW

Official repository:

https://github.com/nigels-com/glew

Official releases:

https://github.com/nigels-com/glew/releases


# Summary

## PC — Windows

Install:

- CMake
- Visual Studio Community 2026 or another compatible MSVC compiler
- OpenGL
- GLEW
- SDL2
- SDL2_image
- SDL2_mixer
- SDL2_ttf

SDL2 and its related libraries are included in the Tridingine repository.

For GLEW:

```text
C:\glew-2.3.1
```

and configure:

```text
CMAKE_PREFIX_PATH=C:\glew-2.3.1
```

Restart the terminal after modifying the environment variables.


## PC — Linux

Install:

- CMake
- GCC or Clang
- OpenGL development libraries
- GLEW
- Required system dependencies

For Debian/Ubuntu:

```bash
sudo apt update
sudo apt install libglew-dev
```

Additional OpenGL development packages may be required depending on the distribution.


## Nintendo 3DS

Install through devkitPro:

- devkitPro
- devkitARM
- libctru
- citro2d
- citro3d
- libogg
- libopus
- libopusfile
- Other required 3DS portlibs

Follow the official instructions:

https://devkitpro.org/wiki/Getting_Started

The Nintendo 3DS dependencies should be installed through devkitPro rather than copied manually into the project.