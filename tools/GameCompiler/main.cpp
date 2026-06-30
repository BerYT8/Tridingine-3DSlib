#include <Tridingine.h>

int app_main(int argc, char *argv[])
{
    S2S_ScreensInit();

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