#include <input.h>
#include "inputIntern.h"


#if defined(PLATFORM_3DS)
#include <3ds.h>

#elif defined(PLATFORM_PC)
#include <SDL2/SDL.h>
#endif

#include "screens/screensValues.h"

static bool in = false;

static u32 oldKeys = 0;

static u32 buildVirtualKeys()
{
    u32 result = 0;

    InputBinding* bindings = inputIntern::getBindings();

    int count = inputIntern::getBindingCount();

    for(int i = 0; i < count; i++)
    {
        if(!bindings[i].binded)
            continue;

        bool pressed = false;

#if defined(PLATFORM_3DS)

        u32 held = hidKeysHeld();

        pressed = (held & bindings[i].platformKey) != 0;

#elif defined(PLATFORM_PC)

        const Uint8* keyboard = SDL_GetKeyboardState(NULL);

        pressed = keyboard[bindings[i].platformKey];

#endif

        if(pressed)
        {
            result |= bindings[i].virtualKey;
        }
    }

    return result;
}

void input_bindKey(InputKey virtualKey, AssignPos pos, PlatformKey platformKey)
{
    if(!in)
        return;

#if defined(PLATFORM_PC)
    if(virtualKey == INPUT_KEY_TOUCH)
    {
        if(platformKey == PC_MOUSE_LEFT_BUTTON)
        {
            inputIntern::setLMBClick(true);
            inputIntern::setRMBClick(false);
        }
        if(platformKey == PC_MOUSE_RIGHT_BUTTON)
        {
            inputIntern::setRMBClick(true);
            inputIntern::setLMBClick(false);
        }
        if(platformKey == PC_MOUSE_BOTH_BUTTONS)
        {
            inputIntern::setLMBClick(true);
            inputIntern::setRMBClick(true);
        }
            return;
    }
    else
    {
        if(platformKey == PC_MOUSE_LEFT_BUTTON || platformKey == PC_MOUSE_RIGHT_BUTTON || platformKey == PC_MOUSE_BOTH_BUTTONS)
            return;
    }
#endif

    // printf("Binded Virtual: %d, with platform: %d.\n", virtualKey, platformKey);
    inputIntern::addBinding(virtualKey, pos, platformKey);
}

bool input_isKeyPressed(InputKey key)
{
    if(!in)
        return false;
    if(key == INPUT_KEY_ANY)
    {
        return inputIntern::getHidKeys().kDown != 0;
    }
    return (inputIntern::getHidKeys().kDown & key) != 0;
}

bool input_isKeyReleased(InputKey key)
{
    if(!in)
        return false;
    if(key == INPUT_KEY_ANY)
    {
        return inputIntern::getHidKeys().kUp != 0;
    }
    return (inputIntern::getHidKeys().kUp & key) != 0;
}

bool input_isKeyDown(InputKey key)
{
    if(!in)
        return false;
    if(key == INPUT_KEY_ANY)
    {
        return inputIntern::getHidKeys().kHeld != 0;
    }
    return (inputIntern::getHidKeys().kHeld & key) != 0;
}

bool input_isKeyUp(InputKey key)
{
    if(!in)
        return false;
    if(key == INPUT_KEY_ANY)
    {
        return inputIntern::getHidKeys().kHeld == 0;
    }
    return (inputIntern::getHidKeys().kHeld & key) == 0;
}

Vec2 input_getTouch()
{
    if(!in)
        return {0,0};
    return inputIntern::getTouch();
}

#if defined(PLATFORM_PC)
bool GetMouseBottom(int xt, int yt, float &outX, float &outY)
{
    //printf("Mouse pos X: %d; Bottom Init X: %f; Bottom Width: %f; Mouse pos Y: %d; Bottom Init Y: %f; Bottom Height: %f.\n", xt, bottomInitialPointX, botWidth, yt, bottomInitialPointY, botHeight);
    if (xt < bottomInitialPointX ||
        xt > bottomInitialPointX + botWidth ||
        yt < bottomInitialPointY ||
        yt > bottomInitialPointY + botHeight)
        return false;

    outX = (xt - bottomInitialPointX)/windowScale;
    outY = (yt - bottomInitialPointY)/windowScale;

    return true;
}
#endif

void input_read()
{
    if(!in)
        return;
#if defined(PLATFORM_3DS)

    hidScanInput();

#elif defined(PLATFORM_PC)

    SDL_PumpEvents();

#endif

    u32 current = buildVirtualKeys();

#if defined(PLATFORM_PC)
    int xt, yt;
    SDL_GetMouseState(&xt, &yt);
    //printf("[MOUSE] Mouse X: %d; Mouse Y: %d.\n", xt, yt);

    float bx, by;
    bool can = GetMouseBottom(xt, yt, bx, by);
    if (can)
    {
        //printf("[ADDED] Added correct position.\n");
        inputIntern::setTouch(vec2_create(bx, by));
    }
    //else
        //printf("[ERROR] Error occurred.\n");
#elif defined(PLATFORM_3DS)

    touchPosition touch;

    hidTouchRead(&touch);

    inputIntern::setTouch(vec2_create(touch.px, touch.py));

#endif

#if defined(PLATFORM_PC)
    bool anyB = (current != 0);

    if (can)
    {
        int mx, my;
        Uint32 mouse = SDL_GetMouseState(&mx, &my);

        bool leftClick  = mouse & SDL_BUTTON(SDL_BUTTON_LEFT);
        bool rightClick = mouse & SDL_BUTTON(SDL_BUTTON_RIGHT);

        bool mousePressed =
            (inputIntern::LMB_BUTTON_CLICK && leftClick) ||
            (inputIntern::RMB_BUTTON_CLICK && rightClick);

        if (mousePressed)
        {
            current |= INPUT_KEY_TOUCH;
        }
    }
    else
    {
        current &= ~INPUT_KEY_TOUCH;
    }

#endif

    inputIntern::setHidKeysDown(current & ~oldKeys);
    inputIntern::setHidKeysHeld(current);
    inputIntern::setHidKeysUp(oldKeys & ~current);

    oldKeys = current;
}

void input_init()
{
    if(in)
        return;
    in = true;
#if defined(PLATFORM_3DS)

    hidInit();

    input_bindKey(INPUT_KEY_A, 0, KEY_A);
    input_bindKey(INPUT_KEY_B, 1, KEY_B);
    input_bindKey(INPUT_KEY_X, 2, KEY_X);
    input_bindKey(INPUT_KEY_Y, 3, KEY_Y);

    input_bindKey(INPUT_KEY_L, 4, KEY_L);
    input_bindKey(INPUT_KEY_R, 5, KEY_R);

    input_bindKey(INPUT_KEY_START, 6, KEY_START);
    input_bindKey(INPUT_KEY_SELECT, 7, KEY_SELECT);

    input_bindKey(INPUT_KEY_DUP, 8, KEY_DUP);
    input_bindKey(INPUT_KEY_DDOWN, 9, KEY_DDOWN);
    input_bindKey(INPUT_KEY_DLEFT, 10, KEY_DLEFT);
    input_bindKey(INPUT_KEY_DRIGHT, 11, KEY_DRIGHT);

    input_bindKey(INPUT_KEY_ZL, 12, KEY_ZL);
    input_bindKey(INPUT_KEY_ZR, 13, KEY_ZR);

    input_bindKey(INPUT_KEY_CSTICK_UP, 14, KEY_CSTICK_UP);
    input_bindKey(INPUT_KEY_CSTICK_DOWN, 15, KEY_CSTICK_DOWN);
    input_bindKey(INPUT_KEY_CSTICK_LEFT, 16, KEY_CSTICK_LEFT);
    input_bindKey(INPUT_KEY_CSTICK_RIGHT, 17, KEY_CSTICK_RIGHT);

    input_bindKey(INPUT_KEY_CPAD_UP, 18, KEY_CPAD_UP);
    input_bindKey(INPUT_KEY_CPAD_DOWN, 19, KEY_CPAD_DOWN);
    input_bindKey(INPUT_KEY_CPAD_LEFT, 20, KEY_CPAD_LEFT);
    input_bindKey(INPUT_KEY_CPAD_RIGHT, 21, KEY_CPAD_RIGHT);

    input_bindKey(INPUT_KEY_TOUCH, 22, KEY_TOUCH);

#elif defined(PLATFORM_PC)

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    input_bindKey(INPUT_KEY_A, 0, SDL_SCANCODE_K);
    input_bindKey(INPUT_KEY_B, 1, SDL_SCANCODE_J);
    input_bindKey(INPUT_KEY_X, 2, SDL_SCANCODE_I);
    input_bindKey(INPUT_KEY_Y, 3, SDL_SCANCODE_U);

    input_bindKey(INPUT_KEY_L, 4, SDL_SCANCODE_Q);
    input_bindKey(INPUT_KEY_R, 5, SDL_SCANCODE_E);

    input_bindKey(INPUT_KEY_START, 6, SDL_SCANCODE_RETURN);
    input_bindKey(INPUT_KEY_SELECT, 7, SDL_SCANCODE_BACKSPACE);

    input_bindKey(INPUT_KEY_DUP, 8, SDL_SCANCODE_UP);
    input_bindKey(INPUT_KEY_DDOWN, 9, SDL_SCANCODE_DOWN);
    input_bindKey(INPUT_KEY_DLEFT, 10, SDL_SCANCODE_LEFT);
    input_bindKey(INPUT_KEY_DRIGHT, 11, SDL_SCANCODE_RIGHT);

    input_bindKey(INPUT_KEY_ZL, 12, SDL_SCANCODE_1);
    input_bindKey(INPUT_KEY_ZR, 13, SDL_SCANCODE_3);

    input_bindKey(INPUT_KEY_CSTICK_UP, 14, SDL_SCANCODE_T);
    input_bindKey(INPUT_KEY_CSTICK_DOWN, 15, SDL_SCANCODE_G);
    input_bindKey(INPUT_KEY_CSTICK_LEFT, 16, SDL_SCANCODE_F);
    input_bindKey(INPUT_KEY_CSTICK_RIGHT, 17, SDL_SCANCODE_H);

    input_bindKey(INPUT_KEY_CPAD_UP, 18, SDL_SCANCODE_W);
    input_bindKey(INPUT_KEY_CPAD_DOWN, 19, SDL_SCANCODE_S);
    input_bindKey(INPUT_KEY_CPAD_LEFT, 20, SDL_SCANCODE_A);
    input_bindKey(INPUT_KEY_CPAD_RIGHT, 21, SDL_SCANCODE_D);

    //input_bindKey(INPUT_KEY_TOUCH, 22, SDL_SCANCODE_SPACE);

    input_bindKey(INPUT_KEY_TOUCH, 0, PC_MOUSE_LEFT_BUTTON);

#endif
}

void input_exit()
{
    if(!in)
        return;
    in = false;
#if defined(PLATFORM_3DS)

    hidExit();

#elif defined(PLATFORM_PC)

    SDL_Quit();

#endif
}