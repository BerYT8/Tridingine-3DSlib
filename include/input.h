#pragma once

#include "maths.h"

typedef unsigned int InputKey;
typedef unsigned int PlatformKey;
typedef unsigned int AssignPos;

#ifdef __cplusplus
extern "C"
{
#endif

/*
    Input abstraction layer

    Estos defines representan botones "virtuales" comunes
    entre Nintendo 3DS y SDL/PC.

    Internamente cada plataforma los traduce a:
    - KEY_* de 3DS
    - SDL_SCANCODE_* en PC
*/

enum
{
    INPUT_KEY_A          = 1 << 0,
    INPUT_KEY_B          = 1 << 1,
    INPUT_KEY_X          = 1 << 2,
    INPUT_KEY_Y          = 1 << 3,

    INPUT_KEY_L          = 1 << 4,
    INPUT_KEY_R          = 1 << 5,

    INPUT_KEY_START      = 1 << 6,
    INPUT_KEY_SELECT     = 1 << 7,

    INPUT_KEY_DUP        = 1 << 8,
    INPUT_KEY_DDOWN      = 1 << 9,
    INPUT_KEY_DLEFT      = 1 << 10,
    INPUT_KEY_DRIGHT     = 1 << 11,

    INPUT_KEY_ZL         = 1 << 12,
    INPUT_KEY_ZR         = 1 << 13,

    INPUT_KEY_CSTICK_UP      = 1 << 14,
    INPUT_KEY_CSTICK_DOWN    = 1 << 15,
    INPUT_KEY_CSTICK_LEFT    = 1 << 16,
    INPUT_KEY_CSTICK_RIGHT   = 1 << 17,

    INPUT_KEY_CPAD_UP        = 1 << 18,
    INPUT_KEY_CPAD_DOWN      = 1 << 19,
    INPUT_KEY_CPAD_LEFT      = 1 << 20,
    INPUT_KEY_CPAD_RIGHT     = 1 << 21,

    INPUT_KEY_TOUCH      = 1 << 22,

    INPUT_KEY_ANY        = 0xFFFFFFFF,

    INPUT_KEY_NONE       = -5
};

#if defined(PLATFORM_PC)
enum
{
    PC_MOUSE_LEFT_BUTTON = -2,
    PC_MOUSE_RIGHT_BUTTON = -3,
    PC_MOUSE_BOTH_BUTTONS = -4,
};
#endif

/*
    Traducción de botones reales de plataforma
    a botones abstractos del engine.
*/
void input_init();

void input_read();

bool input_isKeyPressed(InputKey keycode);
bool input_isKeyReleased(InputKey keycode);
bool input_isKeyDown(InputKey keycode);
bool input_isKeyUp(InputKey keycode);

/*
    Remapping.

    Only works on PC SDL code, not devkitpro functionality.
*/
void input_bindKey(InputKey virtualKey, AssignPos pos, PlatformKey platformKey);

Vec2 input_getTouch();

void input_exit();

#ifdef __cplusplus
}
#endif