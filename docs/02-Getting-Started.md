# First Project

In this document, you will learn how to create and run your first project with **Tridingine-3DSlib**.

---

# Project Structure

After generating the development kit, you will find a structure similar to the following:

```text
Lib/
├── include/
├── lib/
├── templates/
├── examples/
├── tools/
├── romfs/
├── build.bat
├── CMakeLists.txt
├── game.json
└── main.cpp
```

The main project file is `main.cpp`, where the game execution begins.

---

# Including the Library

To use the API, simply include:

```cpp
#include <Tridingine.h>
```

The entire API is designed to be used from **C**, although it can also be used without problems from **C++**.

> There is also a specific C++ wrapper API.

---

# Minimum Game Structure

Every project must have, at minimum, the following structure:

```cpp
#include <Tridingine.h>

int main(int argc, char *argv[])
{
    S2S_ScreensInit();

#if defined(GAME_TITLE)
    const char* title = GAME_TITLE;
    SetWindowTitle(title);
#endif

    // Initialization

    while (S2S_ScreensRunning())
    {
        S2S_BeginFrame();

        // Game logic

        S2S_EndFrame();
    }

    // Cleanup

    S2S_ScreensExit();

    return 0;
}
```

Without this structure, there will be no valid window or properly initialized execution context.

---

# Explanation

## S2S_ScreensInit()

Initializes the screen system.

This function prepares the application to begin running the game.

It should be called only once when the program starts.

---

## S2S_ScreensRunning()

Returns whether the application should continue running.

As long as this function returns `true`, the game will continue running.

When the user closes the window or the application terminates, it will return `false`.

---

## S2S_BeginFrame()

Marks the beginning of a new frame.

All game logic and drawing operations should be performed between `S2S_BeginFrame()` and `S2S_EndFrame()`.

---

## S2S_EndFrame()

Ends the current frame.

This function presents the image on screen and prepares the next frame.

---

## S2S_ScreensExit()

Releases all resources used by the screen system.

It should be called before the program terminates.

---

## SetWindowTitle()

This is a **PC-only function** used to set the window title using the title defined in `game.json`.

---

# Building the Project

To build the Windows version:

```bat
build.bat
```

To build the Nintendo 3DS version:

```bat
build.bat 3ds
```

There is no need to modify the source code when switching platforms.

---

# Next Step

Once the basic project is working correctly, you can continue with [**03 - Graphics**](03-Graphics.md), where you will learn how to draw:

- Rectangles
- Triangles
- Ellipses
- Outlined rectangles
- Lines
- Points
- Text
- Sprites
- And the rest of the graphics primitives available in Tridingine-3DSlib.