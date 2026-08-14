# Installation

This document explains how to build **Tridingine-3DSlib**, generate the development kit, and create ready-to-use projects.

---

# Requirements

Currently, **Tridingine-3DSlib** is primarily designed to run natively according to your operating system. The build workflow supports:

- **Windows**: Native execution using `.bat` files.
- **Linux / macOS**: Native execution using `.sh` files.

Both platforms use the same API and project structure.

---

# Required Software

Before building the library, you need to install the following tools.

## DevkitPro

Install **DevkitPro** using the official installer.

The default installation path varies depending on the operating system:

- **Windows**: `C:\devkitPro`
- **Linux / macOS**: `/opt/devkitpro`

This dependency is required to build projects targeting **Nintendo 3DS**.

---

## CMake

Install the latest version of **CMake** and make sure it is added to your system's PATH so that it can be executed from the command line.

---

# Building Tridingine-3DSlib (Step 1)

The build process consists of two phases. During this first phase, **only the engine's main library** is compiled for the selected platform.

From the repository root directory, run the command corresponding to your operating system:

### Windows

```bat
.\build.bat
```

### Linux / macOS

```bash
chmod +x ./build.sh
./build.sh
```

---

# Generating the Development Kit (Step 2)

Once the main library has been compiled in Step 1, **you must** run the post-build script. This script compiles the `ProjectMaker` tool, all secondary utilities (`PakMaker`, `SoundMaker3DS`, etc.), and packages the final `Lib` folder structure.

From the repository root directory, run:

### Windows

```bat
.\MakeProjectMaker.bat
```

### Linux / macOS

```bash
chmod +x ./MakeProjectMaker.sh
./MakeProjectMaker.sh
```

---

# Generated Folders

After both scripts have finished, you will find the following structure in the repository root:

## build/

Contains temporary configuration files and objects generated during the PC build process.

## ProjectMaker.exe / ProjectMaker

The project creation tool executable is copied directly to the repository root after running the Step 2 script.

## build/Lib/

This is your **Development Kit**, packaged and ready to use. It contains everything required to create games with Tridingine-3DSlib.

---

# Development Kit Structure (build/Lib/)

After successfully completing Step 2 (`MakeProjectMaker`), the `build/Lib/` folder will have the following structure:

```text
build/Lib/
├── include/
├── lib/
│   ├── pc/
│   └── 3ds/
├── templates/
├── examples/
├── tools/
├── content/
└── romfs/
    ├── pc/
    └── 3ds/
```

---

## include/

Public header files (`.h` / `.hpp`) for the library.

---

## lib/

Compiled static libraries required for linking the project (`.lib` for PC, `.a` for 3DS).

---

## templates/

Internal templates used by the environment for project creation.

---

## examples/

Examples demonstrating how to use the Tridingine API.

---

## tools/

Compiled internal tools, such as `PakMaker` and `SoundMaker3DS`, used by the engine ecosystem.

---

## content/

This folder should contain all the game's source assets (textures, audio, fonts, shaders, etc.).

---

## romfs/

Contains the processed and optimized assets ready to be consumed by the executable for each platform (**PC** and **3DS**).

---

# ProjectMaker Tool

`ProjectMaker` is the utility included in the repository root for creating and updating projects based on the development kit generated in `build/Lib/`.

## Usage

### Windows

```bat
ProjectMaker.exe -o [output_folder]
```

### Linux / macOS

```bash
./ProjectMaker -o [output_folder]
```

### Options

```text
ProjectMaker --version/-v
ProjectMaker --update -o <FOLDER>
ProjectMaker -o <FOLDER>
ProjectMaker [--only <pattern> ...]
ProjectMaker [--exclude <pattern> ...]
ProjectMaker [--no-libs]
ProjectMaker [--no-examples]
```

* `--version`, `-v` — Displays the current ProjectMaker version.
* `-o <FOLDER>` — Specifies the destination folder where the project will be created or updated.
* `--update` — Updates an existing project using the latest ProjectMaker templates and development kit files.
* `--only <pattern>` — Only includes files matching the specified pattern. The option can be specified multiple times.
* `--exclude <pattern>` — Excludes files matching the specified pattern. The option can be specified multiple times.
* `--no-libs` — Does not include the bundled Tridingine libraries in the generated project. This is optional when updating a project if Tridingine has already been installed through CMake.
* `--no-examples` — Does not include the examples in the generated project.

### Creating a new project

To create a completely clean project, specify the destination with `-o`:

```bash
ProjectMaker -o [FOLDER]
```

The program will generate a clean project in the specified destination, including the required structure, base CMake configuration, and local build scripts, allowing you to start development immediately.

### Updating an existing project

The `--update` option is intended primarily for updating projects created from the `examples/` and `templates/` directories when a new version of the development kit is available.

For example, to update the `Orbit` example:

```bash
ProjectMaker -o "[FOLDER]/Orbit" --update
```

If you do not want to include the examples in the updated project:

```bash
ProjectMaker -o "[FOLDER]/Orbit" --update --no-examples
```

If you have already installed Tridingine through CMake — that is, you have already built and installed the Tridingine library project — you can also omit the bundled libraries:

```bash
ProjectMaker -o "[FOLDER]/Orbit" --update --no-libs
```

`--no-libs` is optional in this situation. It is harmless to include the libraries even if Tridingine is already installed through CMake; the option simply prevents ProjectMaker from copying the bundled library files into the project.

The same options can be combined:

```bash
ProjectMaker -o "[FOLDER]/Orbit" --update --no-examples --no-libs
```

This updates the project without copying the example files or the bundled Tridingine libraries.

### Result

The program will generate or update the project in the specified destination, providing the required project structure, base CMake configuration, development kit files, and local build scripts. This allows existing projects to be updated to the latest available templates and development kit while preserving their existing project contents.

---

# Building a Generated Project

Projects created with `ProjectMaker` are self-contained and include their own automated scripts. To build your new game:

### PC version

- Windows: `.\build.bat`
- Linux: `./build.sh`

### Nintendo 3DS version

- Windows: `.\build.bat 3ds`
- Linux: `./build.sh build_3ds`

There is no need to modify the game's source code when switching platforms.

---

# Running Directly on Nintendo 3DS Using `3dslink`

Nintendo 3DS projects can be sent directly to a console using **`3dslink`**, allowing you to build and test the game without manually copying the `.3dsx` file to the SD card.

This feature is available by adding `link` to the 3DS build command.

## Requirements

To use this feature, you need:

- **DevkitPro** installed with `3dslink` available in your PATH.
- The **Nintendo 3DS connected to the same Wi-Fi network as the PC**.
- **Homebrew Launcher/Menu open on the 3DS**.
- Inside Homebrew Launcher/Menu, press **`Y`** to enable the **Netloader**.
- Know the 3DS IP address if you want to use a direct connection.

When the Netloader is active, the 3DS waits for a connection from the PC.

> **Important:** The 3DS must be connected to the same local network as the PC. No port forwarding or Internet access to the console is required.

---

# game.json

The `game.json` file in your project contains the information used by the packaging tools to package the game, especially the `.3dsx` file for the console.

Example:

```json
{
    "file": "name_of_file",
    "title": "Game Title",
    "author": "You",
    "description": "Description."
}
```

---

# Next Step

Once your project has been generated with `ProjectMaker`, continue with:

**[02 - Getting Started](02-Getting-Started.md)** to create your first game using **Tridingine-3DSlib**.