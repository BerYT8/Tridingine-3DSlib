# Input

The **Input** module provides an abstraction layer for user input, allowing the same buttons to be used regardless of the platform.

Currently supported:

- Nintendo 3DS
- PC (SDL)

The translation between physical buttons and the engine's buttons is handled automatically.

---

# Initialization

Before using the input system:

```c
input_init();
```

The input state must be updated every frame:

```c
input_read();
```

When the application exits:

```c
input_exit();
```

---

# Virtual Buttons

The engine uses a set of abstract buttons shared across all platforms.

## Main Buttons

```c
INPUT_KEY_A
INPUT_KEY_B
INPUT_KEY_X
INPUT_KEY_Y
```

---

## Triggers

```c
INPUT_KEY_L
INPUT_KEY_R

INPUT_KEY_ZL
INPUT_KEY_ZR
```

---

## System

```c
INPUT_KEY_START
INPUT_KEY_SELECT
```

---

## D-Pad

```c
INPUT_KEY_DUP
INPUT_KEY_DDOWN
INPUT_KEY_DLEFT
INPUT_KEY_DRIGHT
```

---

## Circle Pad

```c
INPUT_KEY_CPAD_UP
INPUT_KEY_CPAD_DOWN
INPUT_KEY_CPAD_LEFT
INPUT_KEY_CPAD_RIGHT
```

---

## C-Stick

```c
INPUT_KEY_CSTICK_UP
INPUT_KEY_CSTICK_DOWN
INPUT_KEY_CSTICK_LEFT
INPUT_KEY_CSTICK_RIGHT
```

---

## Touchscreen

```c
INPUT_KEY_TOUCH
```

---

## Special

Detect any button:

```c
INPUT_KEY_ANY
```

No valid button:

```c
INPUT_KEY_NONE
```

---

# Querying Buttons

## Pressed This Frame

Returns `true` only during the frame in which the button was pressed.

```c
if (input_isKeyPressed(INPUT_KEY_A))
{
    // ...
}
```

---

## Released This Frame

```c
if (input_isKeyReleased(INPUT_KEY_A))
{
    // ...
}
```

---

## Held Down

Returns `true` while the button remains pressed.

```c
if (input_isKeyDown(INPUT_KEY_A))
{
    // ...
}
```

---

## Not Pressed

```c
if (input_isKeyUp(INPUT_KEY_A))
{
    // ...
}
```

---

# Button Remapping

A platform's physical button can be associated with one of the engine's virtual buttons.

```c
input_bindKey(
    INPUT_KEY_A,
    position,
    platformKey
);
```

This allows you to customize the controls without modifying the rest of the game's code.

---

# Touch Input

Get the current touch position:

```c
Vec2 touch = input_getTouch();
```

The returned value corresponds to the coordinates of the platform's touchscreen.

---

# Mouse Buttons (PC)

The PC version also provides constants for mouse buttons.

Left button:

```c
PC_MOUSE_LEFT_BUTTON
```

Right button:

```c
PC_MOUSE_RIGHT_BUTTON
```

Both buttons:

```c
PC_MOUSE_BOTH_BUTTONS
```

These constants are only available when compiling for the PC platform.

---

# Typical Usage

```c
input_init();

while (S2S_ScreensRunning())
{
    input_read();

    if (input_isKeyPressed(INPUT_KEY_START))
    {
        // Open menu
    }

    if (input_isKeyDown(INPUT_KEY_A))
    {
        // Continuous action
    }
}

input_exit();
```

---

# Next Step

You can now receive user input and interact with the game.

Continue with [**05 - Audio**](05-Audio.md), where you will learn how to:

- Initialize the audio system.
- Play sound effects.
- Play music.
- Control volume and playback.