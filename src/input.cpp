#include <input.h>
#include <vector>
#include "inputIntern.h"

typedef struct InputAction 
{
    std::vector<InputKey> keys = {};

    // Tipo de operación utilizada para evaluar las teclas.
    InputAction_Type type = INPUT_ACTION_AND;

    // Estado de la acción del frame anterior.
    bool wasActive = false;

    // Estado de la acción del frame actual.
    bool active = false;

    // La acción ha sido activada este frame.
    bool triggered = false;

    // La acción ha terminado este frame.
    bool released = false;

    // El objeto ha dejado de ser válido.
    bool obsolete = false;

} InputAction;


bool isSingleKey(uint32_t key)
{
    return key != 0 && (key & (key - 1)) == 0;
}


#if defined(PLATFORM_3DS)

#include <3ds.h>

#elif defined(PLATFORM_PC)

#include <SDL2/SDL.h>
#include "SDL_joystick.h"

static SDL_Joystick* joystick = nullptr;

#endif


#include "screens/screensValues.h"


static bool in = false;

static u32 oldKeys = 0;


// ============================================================
// ACTIONS
// ============================================================

static std::vector<InputAction*> actions;


// ------------------------------------------------------------
// Set Action Type
// ------------------------------------------------------------

void input_setAction_type(
    InputAction* action,
    InputAction_Type type
)
{
    if (!action || action->obsolete)
        return;

    action->type = type;
}


// ------------------------------------------------------------
// Create Action
// ------------------------------------------------------------

InputAction* input_createAction()
{
    InputAction* action = new InputAction();

    if (!action)
        return nullptr;

    action->keys.clear();

    action->type = INPUT_ACTION_AND;

    action->wasActive = false;
    action->active = false;
    action->triggered = false;
    action->released = false;

    action->obsolete = false;

    actions.push_back(action);

    return action;
}


// ------------------------------------------------------------
// Add Key
// ------------------------------------------------------------

void input_addKey_action(
    InputAction* action,
    InputKey key
)
{
    if (!action || !in || action->obsolete)
        return;

    /*
        InputAction solo acepta teclas individuales.

        Por ejemplo:

            INPUT_KEY_A       -> válido
            INPUT_KEY_B       -> válido
            INPUT_KEY_A | INPUT_KEY_B -> inválido
            INPUT_KEY_ANY     -> inválido
    */

    if (key <= INPUT_KEY_NONE || !isSingleKey(key))
        return;

    for (size_t i = 0; i < action->keys.size(); i++)
    {
        if (action->keys[i] == key)
            return;
    }

    action->keys.push_back(key);
}


// ------------------------------------------------------------
// Remove Key
// ------------------------------------------------------------

bool input_removeKey_action(
    InputAction* action,
    InputKey key
)
{
    if (!action || !in || action->obsolete)
        return false;

    if (key <= INPUT_KEY_NONE || !isSingleKey(key))
        return false;

    for (size_t i = 0; i < action->keys.size(); i++)
    {
        if (action->keys[i] == key)
        {
            action->keys.erase(
                action->keys.begin() + i
            );

            return true;
        }
    }

    return false;
}


// ------------------------------------------------------------
// Destroy Action
// ------------------------------------------------------------

void input_destroyAction(InputAction* action)
{
    if (!action)
        return;

    for (size_t i = 0; i < actions.size(); i++)
    {
        if (actions[i] == action)
        {
            actions.erase(
                actions.begin() + i
            );

            break;
        }
    }

    delete action;
}


// ------------------------------------------------------------
// Evaluate Action
// ------------------------------------------------------------

static bool actionIsActive(InputAction* action)
{
    if (!action || action->obsolete)
        return false;

    if (action->keys.empty())
        return false;


    int heldKeys = 0;

    for (InputKey key : action->keys)
    {
        if (input_isKeyDown(key))
        {
            heldKeys++;
        }
    }


    switch (action->type)
    {
        // ----------------------------------------------------
        // AND
        //
        // Todas las teclas deben estar pulsadas.
        //
        // A + B
        //
        // A = held
        // B = held
        // => true
        // ----------------------------------------------------

        case INPUT_ACTION_AND:
        {
            return heldKeys ==
                   (int)action->keys.size();
        }


        // ----------------------------------------------------
        // OR
        //
        // Al menos una tecla debe estar pulsada.
        //
        // A + B
        //
        // A = held
        // B = not held
        // => true
        // ----------------------------------------------------

        case INPUT_ACTION_OR:
        {
            return heldKeys > 0;
        }


        // ----------------------------------------------------
        // XOR
        //
        // Exactamente una tecla debe estar pulsada.
        //
        // A + B
        //
        // A = held
        // B = not held
        // => true
        //
        // A = held
        // B = held
        // => false
        // ----------------------------------------------------

        case INPUT_ACTION_XOR:
        {
            return heldKeys == 1;
        }


        default:
            return false;
    }
}


// ------------------------------------------------------------
// Update Actions
// ------------------------------------------------------------

static void updateActions()
{
    for (InputAction* action : actions)
    {
        if (!action || action->obsolete)
            continue;


        /*
            Una acción sin teclas no puede estar activa.
        */

        if (action->keys.empty())
        {
            action->wasActive = false;
            action->active = false;
            action->triggered = false;
            action->released = false;

            continue;
        }


        // Estado anterior.
        action->wasActive = action->active;


        // Calculamos el estado actual según el tipo
        // de acción.
        action->active =
            actionIsActive(action);


        // ----------------------------------------------------
        // TRIGGERED
        //
        // Inactiva -> activa
        // ----------------------------------------------------

        action->triggered =
            action->active &&
            !action->wasActive;


        // ----------------------------------------------------
        // RELEASED
        //
        // Activa -> inactiva
        // ----------------------------------------------------

        action->released =
            !action->active &&
            action->wasActive;
    }
}


// ------------------------------------------------------------
// Action Triggered
// ------------------------------------------------------------

bool input_isActionTriggered(
    InputAction* action
)
{
    if (!in || !action || action->obsolete)
        return false;

    return action->triggered;
}


// ------------------------------------------------------------
// Action Released
// ------------------------------------------------------------

bool input_isActionReleased(
    InputAction* action
)
{
    if (!in || !action || action->obsolete)
        return false;

    return action->released;
}


// ------------------------------------------------------------
// Action Completed
// ------------------------------------------------------------

bool input_isActionCompleted(
    InputAction* action
)
{
    if (!in || !action || action->obsolete)
        return false;

    return action->active;
}


// ============================================================
// INPUT
// ============================================================


static u32 buildVirtualKeys()
{
    u32 result = 0;

    InputBinding* bindings =
        inputIntern::getBindings();

    int count =
        inputIntern::getBindingCount();


    for (int i = 0; i < count; i++)
    {
        if (!bindings[i].binded)
            continue;


        bool pressed = false;


#if defined(PLATFORM_3DS)

        u32 held = hidKeysHeld();

        pressed =
            (held & bindings[i].platformKey) != 0;


#elif defined(PLATFORM_PC)

        const Uint8* keyboard =
            SDL_GetKeyboardState(NULL);

        pressed =
            keyboard[bindings[i].platformKey];

#endif


        if (pressed)
        {
            result |= bindings[i].virtualKey;
        }
    }


    return result;
}


// ------------------------------------------------------------
// Bind Key
// ------------------------------------------------------------

void input_bindKey(
    InputKey virtualKey,
    AssignPos pos,
    PlatformKey platformKey
)
{
    if (!in)
        return;


    if (!isSingleKey(virtualKey) &&
        virtualKey != INPUT_KEY_NONE)
    {
        return;
    }


#if defined(PLATFORM_PC)

    if (virtualKey == INPUT_KEY_TOUCH)
    {
        if (platformKey == PC_MOUSE_LEFT_BUTTON)
        {
            inputIntern::setLMBClick(true);
            inputIntern::setRMBClick(false);
        }


        if (platformKey == PC_MOUSE_RIGHT_BUTTON)
        {
            inputIntern::setRMBClick(true);
            inputIntern::setLMBClick(false);
        }


        if (platformKey == PC_MOUSE_BOTH_BUTTONS)
        {
            inputIntern::setLMBClick(true);
            inputIntern::setRMBClick(true);
        }


        return;
    }
    else
    {
        if (platformKey == PC_MOUSE_LEFT_BUTTON ||
            platformKey == PC_MOUSE_RIGHT_BUTTON ||
            platformKey == PC_MOUSE_BOTH_BUTTONS)
        {
            return;
        }
    }

#endif


    inputIntern::addBinding(
        virtualKey,
        pos,
        platformKey
    );
}


// ============================================================
// KEY STATES
// ============================================================


// ------------------------------------------------------------
// Pressed
// ------------------------------------------------------------

bool input_isKeyPressed(InputKey key)
{
    if (!in)
        return false;

    if (key == INPUT_KEY_ANY)
    {
        return inputIntern::getHidKeys().kDown != 0;
    }

    if (!isSingleKey(key))
        return false;

    return
        (inputIntern::getHidKeys().kDown & key) != 0;
}


// ------------------------------------------------------------
// Released
// ------------------------------------------------------------

bool input_isKeyReleased(InputKey key)
{
    if (!in)
        return false;

    if (key == INPUT_KEY_ANY)
    {
        return inputIntern::getHidKeys().kUp != 0;
    }

    if (!isSingleKey(key))
        return false;

    return
        (inputIntern::getHidKeys().kUp & key) != 0;
}


// ------------------------------------------------------------
// Down / Held
// ------------------------------------------------------------

bool input_isKeyDown(InputKey key)
{
    if (!in)
        return false;

    if (key == INPUT_KEY_ANY)
    {
        return inputIntern::getHidKeys().kHeld != 0;
    }

    if (!isSingleKey(key))
        return false;

    return
        (inputIntern::getHidKeys().kHeld & key) != 0;
}


// ------------------------------------------------------------
// Up
// ------------------------------------------------------------

bool input_isKeyUp(InputKey key)
{
    if (!in)
        return false;

    if (key == INPUT_KEY_ANY)
    {
        return inputIntern::getHidKeys().kHeld == 0;
    }

    if (!isSingleKey(key))
        return false;

    return
        (inputIntern::getHidKeys().kHeld & key) == 0;
}


// ============================================================
// ANALOG / TOUCH
// ============================================================


Vec2 input_getCPad()
{
    if (!in)
        return {0, 0};

    return inputIntern::getCPad();
}


Vec2 input_getTouch()
{
    if (!in)
        return {0, 0};

    return inputIntern::getTouch();
}


// ============================================================
// MOUSE
// ============================================================

#if defined(PLATFORM_PC)

bool GetMouseBottom(
    int xt,
    int yt,
    float& outX,
    float& outY
)
{
    if (xt < bottomInitialPointX ||
        xt > bottomInitialPointX + botWidth ||
        yt < bottomInitialPointY ||
        yt > bottomInitialPointY + botHeight)
    {
        return false;
    }


    outX =
        (xt - bottomInitialPointX) /
        windowScale;


    outY =
        (yt - bottomInitialPointY) /
        windowScale;


    return true;
}

#endif


// ============================================================
// JOYSTICK
// ============================================================


static Vec2 applyJoystickDeadzone(
    float x,
    float y
)
{
    const float deadzone = 0.15f;


    float length =
        sqrtf(x * x + y * y);


    if (length < deadzone)
    {
        return vec2_create(
            0.0f,
            0.0f
        );
    }


    float amount =
        (length - deadzone) /
        (1.0f - deadzone);


    amount =
        clampf(
            amount,
            0.0f,
            1.0f
        );


    x =
        (x / length) *
        amount;


    y =
        (y / length) *
        amount;


    return vec2_create(
        x,
        y
    );
}


// ============================================================
// INPUT READ
// ============================================================


void input_read()
{
    if (!in)
        return;


#if defined(PLATFORM_3DS)

    hidScanInput();


#elif defined(PLATFORM_PC)

    SDL_PumpEvents();

#endif


    u32 current =
        buildVirtualKeys();


    // ========================================================
    // TOUCH
    // ========================================================

#if defined(PLATFORM_PC)

    int xt;
    int yt;


    SDL_GetMouseState(
        &xt,
        &yt
    );


    float bx;
    float by;


    bool can =
        GetMouseBottom(
            xt,
            yt,
            bx,
            by
        );


    if (can)
    {
        inputIntern::setTouch(
            vec2_create(
                bx,
                by
            )
        );
    }


#elif defined(PLATFORM_3DS)

    touchPosition touch;


    hidTouchRead(
        &touch
    );


    inputIntern::setTouch(
        vec2_create(
            touch.px,
            touch.py
        )
    );


    // ========================================================
    // CPAD 3DS
    // ========================================================

    circlePosition cpad;


    hidCircleRead(
        &cpad
    );


    float x =
        clampf(
            cpad.dx / 156.0f,
            -1.0f,
            1.0f
        );


    float y =
        -clampf(
            cpad.dy / 156.0f,
            -1.0f,
            1.0f
        );


    inputIntern::setCPad(
        applyJoystickDeadzone(
            x,
            y
        )
    );

#endif


    // ========================================================
    // PC TOUCH / MOUSE
    // ========================================================

#if defined(PLATFORM_PC)

    if (can)
    {
        int mx;
        int my;


        Uint32 mouse =
            SDL_GetMouseState(
                &mx,
                &my
            );


        bool leftClick =
            mouse &
            SDL_BUTTON(SDL_BUTTON_LEFT);


        bool rightClick =
            mouse &
            SDL_BUTTON(SDL_BUTTON_RIGHT);


        bool mousePressed =
            (inputIntern::LMB_BUTTON_CLICK &&
             leftClick)
            ||
            (inputIntern::RMB_BUTTON_CLICK &&
             rightClick);


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


    // ========================================================
    // KEY STATES
    // ========================================================

    inputIntern::setHidKeysDown(
        current & ~oldKeys
    );


    inputIntern::setHidKeysHeld(
        current
    );


    inputIntern::setHidKeysUp(
        oldKeys & ~current
    );


    oldKeys =
        current;


    // ========================================================
    // ACTIONS
    //
    // Las acciones deben actualizarse después de actualizar
    // kDown, kHeld y kUp.
    // ========================================================

    updateActions();


    // ========================================================
    // PC CPAD
    // ========================================================

#if defined(PLATFORM_PC)

    float x = 0.0f;
    float y = 0.0f;


    if (input_isKeyPressed(INPUT_KEY_CPAD_UP) ||
        input_isKeyDown(INPUT_KEY_CPAD_UP))
    {
        y -= 1.0f;
    }


    if (input_isKeyPressed(INPUT_KEY_CPAD_DOWN) ||
        input_isKeyDown(INPUT_KEY_CPAD_DOWN))
    {
        y += 1.0f;
    }


    if (input_isKeyPressed(INPUT_KEY_CPAD_LEFT) ||
        input_isKeyDown(INPUT_KEY_CPAD_LEFT))
    {
        x -= 1.0f;
    }


    if (input_isKeyPressed(INPUT_KEY_CPAD_RIGHT) ||
        input_isKeyDown(INPUT_KEY_CPAD_RIGHT))
    {
        x += 1.0f;
    }


    if (joystick)
    {
        Sint16 rawX =
            SDL_JoystickGetAxis(
                joystick,
                SDL_CONTROLLER_AXIS_LEFTX
            );


        Sint16 rawY =
            SDL_JoystickGetAxis(
                joystick,
                SDL_CONTROLLER_AXIS_LEFTY
            );


        float xp =
            clampf(
                rawX / 32767.0f,
                -1.0f,
                1.0f
            );


        float yp =
            clampf(
                rawY / 32767.0f,
                -1.0f,
                1.0f
            );


        Vec2 cpad =
            applyJoystickDeadzone(
                xp,
                yp
            );


        if (cpad.x != 0)
            x = xp;


        if (cpad.y != 0)
            y = yp;
    }


    inputIntern::setCPad(
        applyJoystickDeadzone(
            x,
            y
        )
    );

#endif
}


// ============================================================
// INIT
// ============================================================


void input_init()
{
    if (in)
        return;


    in = true;

    oldKeys = 0;


#if defined(PLATFORM_3DS)

    hidInit();


    input_bindKey(
        INPUT_KEY_A,
        0,
        KEY_A
    );

    input_bindKey(
        INPUT_KEY_B,
        1,
        KEY_B
    );

    input_bindKey(
        INPUT_KEY_X,
        2,
        KEY_X
    );

    input_bindKey(
        INPUT_KEY_Y,
        3,
        KEY_Y
    );


    input_bindKey(
        INPUT_KEY_L,
        4,
        KEY_L
    );

    input_bindKey(
        INPUT_KEY_R,
        5,
        KEY_R
    );


    input_bindKey(
        INPUT_KEY_START,
        6,
        KEY_START
    );

    input_bindKey(
        INPUT_KEY_SELECT,
        7,
        KEY_SELECT
    );


    input_bindKey(
        INPUT_KEY_DUP,
        8,
        KEY_DUP
    );

    input_bindKey(
        INPUT_KEY_DDOWN,
        9,
        KEY_DDOWN
    );

    input_bindKey(
        INPUT_KEY_DLEFT,
        10,
        KEY_DLEFT
    );

    input_bindKey(
        INPUT_KEY_DRIGHT,
        11,
        KEY_DRIGHT
    );


    input_bindKey(
        INPUT_KEY_ZL,
        12,
        KEY_ZL
    );

    input_bindKey(
        INPUT_KEY_ZR,
        13,
        KEY_ZR
    );


    input_bindKey(
        INPUT_KEY_CSTICK_UP,
        14,
        KEY_CSTICK_UP
    );

    input_bindKey(
        INPUT_KEY_CSTICK_DOWN,
        15,
        KEY_CSTICK_DOWN
    );

    input_bindKey(
        INPUT_KEY_CSTICK_LEFT,
        16,
        KEY_CSTICK_LEFT
    );

    input_bindKey(
        INPUT_KEY_CSTICK_RIGHT,
        17,
        KEY_CSTICK_RIGHT
    );


    input_bindKey(
        INPUT_KEY_CPAD_UP,
        18,
        KEY_CPAD_UP
    );

    input_bindKey(
        INPUT_KEY_CPAD_DOWN,
        19,
        KEY_CPAD_DOWN
    );

    input_bindKey(
        INPUT_KEY_CPAD_LEFT,
        20,
        KEY_CPAD_LEFT
    );

    input_bindKey(
        INPUT_KEY_CPAD_RIGHT,
        21,
        KEY_CPAD_RIGHT
    );


    input_bindKey(
        INPUT_KEY_TOUCH,
        22,
        KEY_TOUCH
    );


#elif defined(PLATFORM_PC)

    SDL_Init(
        SDL_INIT_VIDEO |
        SDL_INIT_EVENTS
    );


    input_bindKey(
        INPUT_KEY_A,
        0,
        SDL_SCANCODE_K
    );

    input_bindKey(
        INPUT_KEY_B,
        1,
        SDL_SCANCODE_J
    );

    input_bindKey(
        INPUT_KEY_X,
        2,
        SDL_SCANCODE_I
    );

    input_bindKey(
        INPUT_KEY_Y,
        3,
        SDL_SCANCODE_U
    );


    input_bindKey(
        INPUT_KEY_L,
        4,
        SDL_SCANCODE_Q
    );

    input_bindKey(
        INPUT_KEY_R,
        5,
        SDL_SCANCODE_E
    );


    input_bindKey(
        INPUT_KEY_START,
        6,
        SDL_SCANCODE_RETURN
    );

    input_bindKey(
        INPUT_KEY_SELECT,
        7,
        SDL_SCANCODE_BACKSPACE
    );


    input_bindKey(
        INPUT_KEY_DUP,
        8,
        SDL_SCANCODE_UP
    );

    input_bindKey(
        INPUT_KEY_DDOWN,
        9,
        SDL_SCANCODE_DOWN
    );

    input_bindKey(
        INPUT_KEY_DLEFT,
        10,
        SDL_SCANCODE_LEFT
    );

    input_bindKey(
        INPUT_KEY_DRIGHT,
        11,
        SDL_SCANCODE_RIGHT
    );


    input_bindKey(
        INPUT_KEY_ZL,
        12,
        SDL_SCANCODE_1
    );

    input_bindKey(
        INPUT_KEY_ZR,
        13,
        SDL_SCANCODE_3
    );


    input_bindKey(
        INPUT_KEY_CSTICK_UP,
        14,
        SDL_SCANCODE_T
    );

    input_bindKey(
        INPUT_KEY_CSTICK_DOWN,
        15,
        SDL_SCANCODE_G
    );

    input_bindKey(
        INPUT_KEY_CSTICK_LEFT,
        16,
        SDL_SCANCODE_F
    );

    input_bindKey(
        INPUT_KEY_CSTICK_RIGHT,
        17,
        SDL_SCANCODE_H
    );


    input_bindKey(
        INPUT_KEY_CPAD_UP,
        18,
        SDL_SCANCODE_W
    );

    input_bindKey(
        INPUT_KEY_CPAD_DOWN,
        19,
        SDL_SCANCODE_S
    );

    input_bindKey(
        INPUT_KEY_CPAD_LEFT,
        20,
        SDL_SCANCODE_A
    );

    input_bindKey(
        INPUT_KEY_CPAD_RIGHT,
        21,
        SDL_SCANCODE_D
    );


    input_bindKey(
        INPUT_KEY_TOUCH,
        0,
        PC_MOUSE_LEFT_BUTTON
    );

#endif
}


// ============================================================
// EXIT
// ============================================================


void input_exit()
{
    if (!in)
        return;


    in = false;


    /*
        Las acciones se invalidan antes de limpiar el vector.
        No las destruimos aquí porque siguen siendo propiedad
        del usuario y deben liberarse con input_destroyAction().
    */

    for (InputAction* action : actions)
    {
        if (!action)
            continue;

        action->obsolete = true;

        action->keys.clear();

        action->active = false;
        action->wasActive = false;
        action->triggered = false;
        action->released = false;
    }


    actions.clear();


#if defined(PLATFORM_3DS)

    hidExit();


#elif defined(PLATFORM_PC)

    SDL_Quit();

#endif
}
