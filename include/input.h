#pragma once

#include "maths.h"

typedef unsigned int InputKey;
typedef unsigned int PlatformKey;
typedef unsigned int AssignPos;

/**
 * @brief Defines how the keys belonging to an InputAction are evaluated.
 *
 * The action evaluates its keys according to the selected operation:
 *
 * - INPUT_ACTION_AND: all keys must be held.
 * - INPUT_ACTION_OR: at least one key must be held.
 * - INPUT_ACTION_XOR: exactly one key must be held.
 *
 * The default type for newly created actions is INPUT_ACTION_AND.
 */
typedef enum InputAction_Type
{
    /** @brief All keys must be held simultaneously. */
    INPUT_ACTION_AND,

    /** @brief At least one key must be held. */
    INPUT_ACTION_OR,

    /** @brief Exactly one key must be held. */
    INPUT_ACTION_XOR

} InputAction_Type;


typedef struct InputAction InputAction;

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @file input.h
 *
 * @brief Cross-platform input abstraction layer.
 *
 * Provides a common input interface for all supported platforms.
 *
 * The engine uses virtual input keys instead of accessing platform-specific
 * input APIs directly. Each platform translates its physical input into
 * these virtual keys.
 *
 * Supported platforms currently include:
 * - Nintendo 3DS
 * - PC / SDL2
 *
 * The input system provides:
 * - Virtual keyboard/controller buttons.
 * - Key pressed, released and held states.
 * - Input bindings.
 * - Input actions and key combinations.
 * - Circle Pad / analog input.
 * - Touch / mouse input.
 */


/**
 * @brief Virtual input keys.
 *
 * These values represent abstract buttons understood by the engine.
 *
 * Each key is represented by exactly one bit, allowing several keys to be
 * combined into a single bit mask using the bitwise OR operator.
 *
 * Example:
 *
 * @code
 * InputKey keys = INPUT_KEY_A | INPUT_KEY_B;
 * @endcode
 *
 * Platform-specific input is translated to these values internally:
 * - Nintendo 3DS uses KEY_* values from libctru.
 * - PC uses SDL scancodes and mouse buttons.
 */
enum
{
    /** @brief Virtual A button. */
    INPUT_KEY_A = 1 << 0,

    /** @brief Virtual B button. */
    INPUT_KEY_B = 1 << 1,

    /** @brief Virtual X button. */
    INPUT_KEY_X = 1 << 2,

    /** @brief Virtual Y button. */
    INPUT_KEY_Y = 1 << 3,


    /** @brief Virtual left shoulder button. */
    INPUT_KEY_L = 1 << 4,

    /** @brief Virtual right shoulder button. */
    INPUT_KEY_R = 1 << 5,


    /** @brief Virtual START button. */
    INPUT_KEY_START = 1 << 6,

    /** @brief Virtual SELECT button. */
    INPUT_KEY_SELECT = 1 << 7,


    /** @brief Virtual D-Pad up. */
    INPUT_KEY_DUP = 1 << 8,

    /** @brief Virtual D-Pad down. */
    INPUT_KEY_DDOWN = 1 << 9,

    /** @brief Virtual D-Pad left. */
    INPUT_KEY_DLEFT = 1 << 10,

    /** @brief Virtual D-Pad right. */
    INPUT_KEY_DRIGHT = 1 << 11,


    /** @brief Virtual ZL shoulder button. */
    INPUT_KEY_ZL = 1 << 12,

    /** @brief Virtual ZR shoulder button. */
    INPUT_KEY_ZR = 1 << 13,


    /** @brief Virtual Circle Stick up direction. */
    INPUT_KEY_CSTICK_UP = 1 << 14,

    /** @brief Virtual Circle Stick down direction. */
    INPUT_KEY_CSTICK_DOWN = 1 << 15,

    /** @brief Virtual Circle Stick left direction. */
    INPUT_KEY_CSTICK_LEFT = 1 << 16,

    /** @brief Virtual Circle Stick right direction. */
    INPUT_KEY_CSTICK_RIGHT = 1 << 17,


    /** @brief Virtual Circle Pad up direction. */
    INPUT_KEY_CPAD_UP = 1 << 18,

    /** @brief Virtual Circle Pad down direction. */
    INPUT_KEY_CPAD_DOWN = 1 << 19,

    /** @brief Virtual Circle Pad left direction. */
    INPUT_KEY_CPAD_LEFT = 1 << 20,

    /** @brief Virtual Circle Pad right direction. */
    INPUT_KEY_CPAD_RIGHT = 1 << 21,


    /**
     * @brief Virtual touch input.
     *
     * On Nintendo 3DS this corresponds to the touchscreen.
     * On PC this is mapped to the configured mouse button.
     */
    INPUT_KEY_TOUCH = 1 << 22,


    /**
     * @brief Special key representing any input.
     *
     * This value is not a physical key and should not be used when creating
     * an InputAction.
     *
     * It can be used with the key state functions to check whether any
     * input was pressed, released or is currently held.
     */
    INPUT_KEY_ANY = 0xFFFFFFFF,


    /**
     * @brief Represents no input key.
     *
     * Used to clear an input binding or indicate that no key is assigned.
     */
    INPUT_KEY_NONE = 0
};


#if defined(PLATFORM_PC)

/**
 * @brief Special platform key representing the left mouse button.
 *
 * These values are used by input_bindKey() when binding virtual input
 * keys to mouse buttons on PC.
 */
enum
{
    /** @brief Left mouse button. */
    PC_MOUSE_LEFT_BUTTON = -2,

    /** @brief Right mouse button. */
    PC_MOUSE_RIGHT_BUTTON = -3,

    /** @brief Both left and right mouse buttons. */
    PC_MOUSE_BOTH_BUTTONS = -4
};

#endif


/**
 * @brief Initializes the input system.
 *
 * Initializes the input backend for the current platform and creates the
 * default input bindings.
 *
 * This function must be called before using any other input function.
 *
 * @code
 * input_init();
 * @endcode
 */
void input_init();


/**
 * @brief Updates the input state.
 *
 * Reads the current state from the platform and updates:
 * - Pressed keys.
 * - Held keys.
 * - Released keys.
 * - Input actions.
 * - Touch position.
 * - Circle Pad position.
 *
 * This function should normally be called once per frame before processing
 * gameplay input.
 *
 * @code
 * while (running)
 * {
 *     input_read();
 *
 *     // Game logic...
 * }
 * @endcode
 */
void input_read();


/**
 * @brief Creates a new input action.
 *
 * An InputAction represents one or more virtual keys that must be held
 * simultaneously.
 *
 * For example:
 *
 * @code
 * InputAction* action = input_createAction();
 *
 * input_addKey_action(action, INPUT_KEY_A);
 * input_addKey_action(action, INPUT_KEY_B);
 * @endcode
 *
 * The resulting action represents the combination:
 *
 *     A + B
 *
 * The returned action is dynamically allocated and must eventually be
 * released with input_destroyAction().
 *
 * @return Pointer to the newly created InputAction.
 * @return nullptr if the action could not be created.
 */
InputAction* input_createAction();


/**
 * @brief Sets the evaluation type of an input action.
 *
 * Determines how the keys contained in the action are combined.
 *
 * @param action Input action to modify.
 * @param type Evaluation type to use.
 *
 * Newly created actions use INPUT_ACTION_AND by default.
 */
void input_setAction_type(InputAction* action, InputAction_Type type);


/**
 * @brief Adds a virtual key to an input action.
 *
 * The key is added to the action's combination.
 *
 * Duplicate keys are ignored.
 *
 * Only single-bit virtual keys are accepted. INPUT_KEY_ANY and combined
 * bit masks cannot be added to an action.
 *
 * Example:
 *
 * @code
 * InputAction* action = input_createAction();
 *
 * input_addKey_action(action, INPUT_KEY_A);
 * input_addKey_action(action, INPUT_KEY_B);
 * @endcode
 *
 * The action now requires both A and B to be held.
 *
 * @param action Input action to modify.
 * @param key Virtual key to add.
 */
void input_addKey_action(InputAction* action, InputKey key);


/**
 * @brief Removes a virtual key from an input action.
 *
 * If the key exists in the action, it is removed.
 *
 * @param action Input action to modify.
 * @param key Virtual key to remove.
 *
 * @return true if the key was found and removed.
 * @return false if the action is invalid or the key was not found.
 */
bool input_removeKey_action(InputAction* action, InputKey key);


/**
 * @brief Destroys an input action.
 *
 * Releases all resources associated with the action.
 *
 * The pointer must not be used after this function returns.
 *
 * @param action Input action to destroy.
 */
void input_destroyAction(InputAction* action);


/**
 * @brief Checks whether an action was triggered this frame.
 *
 * An action is triggered when it changes from inactive to active.
 *
 * For an action containing multiple keys, all keys must be held for the
 * action to become active.
 *
 * Example:
 *
 * @code
 * if (input_isActionTriggered(jumpAction))
 * {
 *     player_jump();
 * }
 * @endcode
 *
 * This function returns true for one frame when the action becomes active.
 *
 * @param action Input action to check.
 *
 * @return true if the action was activated this frame.
 * @return false otherwise.
 */
bool input_isActionTriggered(InputAction* action);


/**
 * @brief Checks whether an action was released this frame.
 *
 * An action is considered released when it was active during the previous
 * frame and is no longer active during the current frame.
 *
 * For actions containing multiple keys, releasing any required key causes
 * the action to become inactive.
 *
 * @param action Input action to check.
 *
 * @return true if the action was released this frame.
 * @return false otherwise.
 */
bool input_isActionReleased(InputAction* action);


/**
 * @brief Checks whether an action is currently completed/active.
 *
 * Returns true while all keys belonging to the action are currently held.
 *
 * Unlike input_isActionTriggered(), this function remains true for every
 * frame while the action remains active.
 *
 * @param action Input action to check.
 *
 * @return true if all action keys are currently held.
 * @return false otherwise.
 */
bool input_isActionCompleted(InputAction* action);


/**
 * @brief Checks whether a key was pressed this frame.
 *
 * A key is considered pressed only during the frame in which it transitions
 * from not held to held.
 *
 * Example:
 *
 * @code
 * if (input_isKeyPressed(INPUT_KEY_A))
 * {
 *     // A was pressed this frame.
 * }
 * @endcode
 *
 * INPUT_KEY_ANY can be used to check whether any virtual key was pressed.
 *
 * @param keycode Virtual input key to check.
 *
 * @return true if the key was pressed this frame.
 * @return false otherwise.
 */
bool input_isKeyPressed(InputKey keycode);


/**
 * @brief Checks whether a key was released this frame.
 *
 * A key is considered released only during the frame in which it transitions
 * from held to not held.
 *
 * INPUT_KEY_ANY can be used to check whether any virtual key was released.
 *
 * @param keycode Virtual input key to check.
 *
 * @return true if the key was released this frame.
 * @return false otherwise.
 */
bool input_isKeyReleased(InputKey keycode);


/**
 * @brief Checks whether a key is currently held.
 *
 * Returns true every frame while the specified key remains pressed.
 *
 * INPUT_KEY_ANY can be used to check whether at least one virtual key
 * is currently held.
 *
 * @param keycode Virtual input key to check.
 *
 * @return true if the key is currently held.
 * @return false otherwise.
 */
bool input_isKeyDown(InputKey keycode);


/**
 * @brief Checks whether a key is currently up.
 *
 * Returns true when the specified key is not currently held.
 *
 * INPUT_KEY_ANY can be used to check whether no virtual key is currently
 * held.
 *
 * @param keycode Virtual input key to check.
 *
 * @return true if the key is currently not held.
 * @return false otherwise.
 */
bool input_isKeyUp(InputKey keycode);


/**
 * @brief Creates a binding between a virtual key and a platform key.
 *
 * A virtual key represents an engine-level input, while a platform key
 * represents the physical input used to activate it.
 *
 * Examples:
 *
 * Nintendo 3DS:
 *
 * @code
 * input_bindKey(INPUT_KEY_A, 0, KEY_A);
 * @endcode
 *
 * PC:
 *
 * @code
 * input_bindKey(INPUT_KEY_A, 0, SDL_SCANCODE_K);
 * @endcode
 *
 * The AssignPos value identifies the binding slot.
 *
 * @param virtualKey Virtual input key used by the engine.
 * @param pos Binding slot.
 * @param platformKey Platform-specific input code.
 */
void input_bindKey(
    InputKey virtualKey,
    AssignPos pos,
    PlatformKey platformKey
);


/**
 * @brief Returns the current Circle Pad direction.
 *
 * The returned vector is normalized and represents the current analog
 * direction.
 *
 * X axis:
 * - -1.0 = left
 * -  0.0 = center
 * -  1.0 = right
 *
 * Y axis:
 * - -1.0 = up
 * -  0.0 = center
 * -  1.0 = down
 *
 * A deadzone is applied internally to prevent small analog movements
 * caused by controller noise.
 *
 * @return Current Circle Pad direction as a Vec2.
 */
Vec2 input_getCPad();


/**
 * @brief Returns the current touch position.
 *
 * On Nintendo 3DS this represents the touchscreen position.
 *
 * On PC, the mouse position is converted into the coordinates of the
 * emulated bottom screen when the mouse is inside that area.
 *
 * @return Current touch position as a Vec2.
 */
Vec2 input_getTouch();


/**
 * @brief Shuts down the input system.
 *
 * Releases resources associated with the platform-specific input backend.
 *
 * After calling this function, input_read() and the other input functions
 * will no longer return valid input states until input_init() is called
 * again.
 *
 * Existing InputAction objects are not destroyed automatically and remain
 * the responsibility of the caller.
 */
void input_exit();


#ifdef __cplusplus
}
#endif
