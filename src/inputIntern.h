#pragma once

#include <fcntl.h>
#include <input.h>

#define MAX_BINDINGS 64

#if defined(PLATFORM_PC)
#include <SDL2/SDL.h>
typedef unsigned int PlatformKey;
typedef unsigned int u32;

#elif defined(PLATFORM_3DS)
#include <3ds.h>
#endif

typedef unsigned int InputKey;

struct hidKeys
{
    u32 kDown = 0;
    u32 kDownR = 0;
    u32 kHeld = 0;
    u32 kUp = 0;
};

struct InputBinding
{
    bool binded = false;
    InputKey virtualKey;
    PlatformKey platformKey;
};

class inputIntern
{
private:

    inline static hidKeys hidKeysI;

    inline static Vec2 touch;
    inline static Vec2 cpadInput;

    inline static InputBinding bindings[MAX_BINDINGS];

public:
#if defined(PLATFORM_PC)
    inline static bool RMB_BUTTON_CLICK = true;
    inline static bool LMB_BUTTON_CLICK = true;

    static void setLMBClick(bool b)
    {
        LMB_BUTTON_CLICK = b;
    }
    
    static void setRMBClick(bool b)
    {
        RMB_BUTTON_CLICK = b;
    }
#endif

    static hidKeys getHidKeys()
    {
        return hidKeysI;
    }

    static void setHidKeysDown(u32 key)
    {
        hidKeysI.kDown = key;
    }

    static void setHidKeysDownR(u32 key)
    {
        hidKeysI.kDownR = key;
    }
    static void setHidKeysHeld(u32 key)
    {
        hidKeysI.kHeld = key;
    }

    static void setHidKeysUp(u32 key)
    {
        hidKeysI.kUp = key;
    }

    static void setTouch(Vec2 t)
    {
        touch = t;
    }

    static Vec2 getTouch()
    {
        return touch;
    }

    static void setCPad(Vec2 cpad)
    {
        cpadInput = cpad;
    }

    static Vec2 getCPad()
    {
        return cpadInput;
    }

    static void addBinding(InputKey virtualKey, AssignPos pos, PlatformKey platformKey)
    {
        if(pos >= MAX_BINDINGS)
            return;
            
        if(virtualKey <= INPUT_KEY_NONE)
        {
            bindings[pos] = {};
            return;
        }

        bindings[pos] =
        {
            true,
            virtualKey,
            platformKey
        };
    }

    static InputBinding getBinding(AssignPos pos)
    {
        if(pos >= MAX_BINDINGS)
            return InputBinding();
        return bindings[pos];
    }

    static InputBinding* getBindings()
    {
        return bindings;
    }

    static int getBindingCount()
    {
        return MAX_BINDINGS;
    }
};