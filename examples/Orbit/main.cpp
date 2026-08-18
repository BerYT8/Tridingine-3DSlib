#include <Tridingine.h>

#include "Game.h"
#include "input.h"

int app_main()
{
    S2S_ScreensInit();

#if defined(GAME_TITLE)
    SetWindowTitle(GAME_TITLE);
#endif

    if (!D2D_Init())
    {
        S2S_ScreensExit();
        return -1;
    }

    D2D_Prepare();

    input_init();

    Game game;

    if (!game.Init())
    {
        input_exit();
        D2D_Exit();
        S2S_ScreensExit();
        return -1;
    }
    dt_init();

    while (S2S_ScreensRunning())
    {
        dt_update();

        input_read();

        if(input_isKeyPressed(INPUT_KEY_A) || input_isKeyPressed(INPUT_KEY_B) || input_isKeyPressed(INPUT_KEY_X) || input_isKeyPressed(INPUT_KEY_Y))
        {
            S2S_StopRunning();
        }

        S2S_BeginFrame();

        if(input_isKeyPressed(INPUT_KEY_SELECT) || input_isKeyPressed(INPUT_KEY_TOUCH))
        {
            S2S_SetGamePaused(!S2S_IsGamePaused());
        }

        S2S_SetCurrentScreen(TOP);

        if(!S2S_IsGamePaused())
        {
            game.Update();
        }

        game.DrawTop();

        S2S_SetCurrentScreen(BOTTOM);

        game.DrawBot();

        S2S_EndFrame();
    }

    game.Exit();

    input_exit();
    D2D_Exit();

    S2S_ScreensExit();

    return 0;
}