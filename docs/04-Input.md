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

Returns `true` only during the frame in which the button was released.

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

Returns `true` while the button is not currently pressed.

```c
if (input_isKeyUp(INPUT_KEY_A))
{
    // ...
}
```

---

# Input Actions

**Input Actions** provide a higher-level way of handling input.

An action contains one or more virtual buttons and has an **action type** that determines how those buttons are evaluated.

The available action types are:

- `INPUT_ACTION_AND`
- `INPUT_ACTION_OR`
- `INPUT_ACTION_XOR`

The default type is:

```c
INPUT_ACTION_AND
```

---

# Action Types

## AND

`INPUT_ACTION_AND` requires **all** keys in the action to be held simultaneously.

This is the default action type.

For example:

```text
A + B
```

The action is active only when both `A` and `B` are held.

```c
InputAction* action = input_createAction();

input_addKey_action(
    action,
    INPUT_KEY_A
);

input_addKey_action(
    action,
    INPUT_KEY_B
);
```

It can also be set explicitly:

```c
input_setAction_type(
    action,
    INPUT_ACTION_AND
);
```

---

## OR

`INPUT_ACTION_OR` requires **at least one** of the keys in the action to be held.

For example:

```text
A OR B
```

The action becomes active when either `A`, `B`, or both are held.

```c
InputAction* action = input_createAction();

input_addKey_action(
    action,
    INPUT_KEY_A
);

input_addKey_action(
    action,
    INPUT_KEY_B
);

input_setAction_type(
    action,
    INPUT_ACTION_OR
);
```

The action is active in all of these cases:

```text
A = pressed    B = released    → Active
A = released   B = pressed      → Active
A = pressed    B = pressed      → Active
A = released   B = released     → Inactive
```

---

## XOR

`INPUT_ACTION_XOR` requires **exactly one** of the keys to be held.

For example:

```text
A XOR B
```

The action is active when exactly one of the two buttons is held.

```c
InputAction* action = input_createAction();

input_addKey_action(
    action,
    INPUT_KEY_A
);

input_addKey_action(
    action,
    INPUT_KEY_B
);

input_setAction_type(
    action,
    INPUT_ACTION_XOR
);
```

The action behaves as follows:

```text
A = pressed    B = released    → Active
A = released   B = pressed      → Active
A = pressed    B = pressed      → Inactive
A = released   B = released     → Inactive
```

---

# Creating an Action

Create an empty action using:

```c
InputAction* action = input_createAction();
```

A newly created action uses:

```c
INPUT_ACTION_AND
```

by default.

Keys can then be added to the action:

```c
input_addKey_action(
    action,
    INPUT_KEY_A
);
```

For a combination:

```c
InputAction* action = input_createAction();

input_addKey_action(
    action,
    INPUT_KEY_A
);

input_addKey_action(
    action,
    INPUT_KEY_B
);
```

Because the default type is `INPUT_ACTION_AND`, the action represents:

```text
A + B
```

Both buttons must be held for the action to become active.

---

# Changing the Action Type

The action type can be changed with:

```c
input_setAction_type(
    action,
    INPUT_ACTION_OR
);
```

For example:

```c
InputAction* action = input_createAction();

input_addKey_action(
    action,
    INPUT_KEY_A
);

input_addKey_action(
    action,
    INPUT_KEY_B
);

input_setAction_type(
    action,
    INPUT_ACTION_OR
);
```

The action is now active when either `A` or `B` is held.

Available types:

```c
INPUT_ACTION_AND
INPUT_ACTION_OR
INPUT_ACTION_XOR
```

---

# Removing a Key

A key can be removed from an action:

```c
input_removeKey_action(
    action,
    INPUT_KEY_B
);
```

The function returns `true` if the key was part of the action and was successfully removed.

---

# Destroying an Action

Actions are dynamically allocated and must be destroyed when they are no longer needed:

```c
input_destroyAction(action);
```

---

# Action States

Input Actions provide three states:

- **Triggered**
- **Completed**
- **Released**

> Note: `input_isActionCompleted()` represents an action that is currently active. It remains `true` while the action's condition is satisfied.

The meaning of "active" depends on the action type.

---

## Triggered

`input_isActionTriggered()` returns `true` for the frame in which the action becomes active.

For a single-key action:

```c
InputAction* action = input_createAction();

input_addKey_action(
    action,
    INPUT_KEY_A
);
```

Then:

```c
if (input_isActionTriggered(action))
{
    // Action started this frame
}
```

The function returns `true` only once when the action transitions from inactive to active.

This works with all action types:

- `INPUT_ACTION_AND`
- `INPUT_ACTION_OR`
- `INPUT_ACTION_XOR`

---

## Completed

`input_isActionCompleted()` returns `true` while the action is currently active.

For example:

```c
if (input_isActionCompleted(action))
{
    // Action is currently active
}
```

The condition depends on the selected action type.

### AND

All keys must be held:

```text
A + B
```

### OR

At least one key must be held:

```text
A OR B
```

### XOR

Exactly one key must be held:

```text
A XOR B
```

---

## Released

`input_isActionReleased()` returns `true` when an active action becomes inactive.

For example:

```c
if (input_isActionReleased(action))
{
    // Action ended this frame
}
```

The transition depends on the action type.

For an `AND` action:

```text
A + B pressed → Active
A + B held    → Active
B released    → Inactive
```

For an `OR` action:

```text
A pressed     → Active
A + B held    → Active
A released    → Active if B remains held
B released    → Inactive
```

For an `XOR` action:

```text
A pressed     → Active
B pressed too → Inactive
A released    → Active if B remains held
```

---

# Action Lifecycle

An action follows the same general lifecycle regardless of its type:

```text
Inactive
   │
   │ action condition becomes true
   ▼
Triggered
   │
   ▼
Completed
   │
   │ action condition becomes false
   ▼
Released
   │
   ▼
Inactive
```

For example, with an `AND` action containing `A + B`:

```text
Frame 1: nothing
Frame 2: A + B pressed  → Triggered + Completed
Frame 3: A + B held     → Completed
Frame 4: A + B held     → Completed
Frame 5: B released     → Released
Frame 6: nothing
```

With an `OR` action containing `A + B`:

```text
Frame 1: nothing
Frame 2: A pressed      → Triggered + Completed
Frame 3: A + B held     → Completed
Frame 4: A released     → Completed (B is still held)
Frame 5: B released     → Released
```

With an `XOR` action containing `A + B`:

```text
Frame 1: nothing
Frame 2: A pressed      → Triggered + Completed
Frame 3: A + B held     → Released
Frame 4: A released     → Triggered + Completed
Frame 5: B released     → Released
```

---

# Example: Single Button Action

Instead of checking the button directly:

```c
if (input_isKeyPressed(INPUT_KEY_A))
{
    player_jump();
}
```

You can create an action:

```c
InputAction* jumpAction = input_createAction();

input_addKey_action(
    jumpAction,
    INPUT_KEY_A
);
```

Then use:

```c
if (input_isActionTriggered(jumpAction))
{
    player_jump();
}
```

The default `INPUT_ACTION_AND` type is sufficient for a single-key action.

---

# Example: Button Combination

Actions can contain multiple keys.

For example, create an action for:

```text
A + B
```

```c
InputAction* specialAction = input_createAction();

input_addKey_action(
    specialAction,
    INPUT_KEY_A
);

input_addKey_action(
    specialAction,
    INPUT_KEY_B
);
```

The default action type is `INPUT_ACTION_AND`, so the action becomes active only while both buttons are pressed:

```c
if (input_isActionTriggered(specialAction))
{
    // Execute special attack
}
```

And it can be continuously checked:

```c
if (input_isActionCompleted(specialAction))
{
    // A and B are currently held
}
```

When one of the buttons is released:

```c
if (input_isActionReleased(specialAction))
{
    // Combination ended
}
```

---

# Example: Multiple Buttons With OR

An action can accept multiple buttons as alternative inputs.

For example, allow either `A` or `B` to perform the same action:

```c
InputAction* jumpAction = input_createAction();

input_addKey_action(
    jumpAction,
    INPUT_KEY_A
);

input_addKey_action(
    jumpAction,
    INPUT_KEY_B
);

input_setAction_type(
    jumpAction,
    INPUT_ACTION_OR
);
```

Now either button can trigger the action:

```c
if (input_isActionTriggered(jumpAction))
{
    player_jump();
}
```

This is useful for providing multiple control options for the same gameplay command.

---

# Example: XOR Action

An XOR action requires exactly one input to be active.

For example:

```c
InputAction* action = input_createAction();

input_addKey_action(
    action,
    INPUT_KEY_A
);

input_addKey_action(
    action,
    INPUT_KEY_B
);

input_setAction_type(
    action,
    INPUT_ACTION_XOR
);
```

The action is active when exactly one of the buttons is held:

```text
A = pressed    B = released    → Active
A = released   B = pressed      → Active
A = pressed    B = pressed      → Inactive
A = released   B = released     → Inactive
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

Input Actions automatically use the remapped virtual buttons, so gameplay code does not need to know which physical key or button is being used.

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

InputAction* jumpAction = input_createAction();

input_addKey_action(
    jumpAction,
    INPUT_KEY_A
);

while (S2S_ScreensRunning())
{
    input_read();

    if (input_isActionTriggered(jumpAction))
    {
        // Jump
    }

    if (input_isActionCompleted(jumpAction))
    {
        // A is still being held
    }

    if (input_isActionReleased(jumpAction))
    {
        // A was released
    }
}

input_destroyAction(jumpAction);

input_exit();
```

---

# Next Step

You can now receive user input, create input actions with `AND`, `OR`, and `XOR` logic, and interact with the game.

Continue with [**05 - Audio**](05-Audio.md), where you will learn how to:

- Initialize the audio system.
- Play sound effects.
- Play music.
- Control volume and playback.
