#include <Tridingine.h>

int app_main(int argc, char *argv[])
{
    S2S_ScreensInit();
#if defined(GAME_TITLE)
    const char* title = GAME_TITLE;
    SetWindowTitle(title);
#endif

    // Inicializaciones

    while (S2S_ScreensRunning())
    {
        S2S_BeginFrame();

        // Lógica
        
        S2S_EndFrame();
    }

    // Salidas

    S2S_ScreensExit();

    return 0;
}