#define ALLOCATE_SHMEM
#include "screensValues.h"

#include <pak_loader/pak_loader.h>

#include "../textures/textures_types.h"

#include "screensLoadingStart.h"
#undef ALLOCATE_SHMEM
#include "../draw/2d/2d_vals.h"
#define ALLOCATE_SHMEM

void S2S_WaitTime(float seconds)
{
#if defined(PLATFORM_PC)
    // 1 segundo = 1000 milisegundos (Convertimos float a entero Uint32)
    Uint32 miliseconds = (Uint32)(seconds * 1000.0f);
    SDL_Delay(miliseconds);

#elif defined(PLATFORM_3DS)
    // 1 segundo = 1,000,000,000 nanosegundos (Usamos un entero de 64 bits)
    s64 nanoseconds = (s64)(seconds * 1000000000.0f);
    svcSleepThread(nanoseconds); 
#endif
}

#if defined(PLATFORM_3DS)
#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

static aptHookCookie hookCookie;

void SystemCallback(APT_HookType hook, void *param)
{
    switch (hook)
    {
        case APTHOOK_ONSUSPEND:
            consoleGamePaused = true;
            break;

        case APTHOOK_ONRESTORE:
            consoleGamePaused = false;
            break;

        case APTHOOK_ONSLEEP:
            closedCover = true;
            consoleGamePaused = true;
            break;

        case APTHOOK_ONWAKEUP:
            closedCover = false;
            consoleGamePaused = false;
            break;
        default:
            break;
    }
}

#endif
#include <color.h>
#include <maths.h>
#include <string.h>

static bool running = false;
static bool restoringWindowState = false;

#if defined(PLATFORM_3DS)
C3D_RenderTarget* top;
C3D_RenderTarget* bottom;
#endif

bool S2S_IsCoverClosed()
{
    return closedCover;
}

bool PlatformPC()
{
#if defined(PLATFORM_PC)
    return true;
#endif
    return false;
}
bool Platform3DS()
{
#if defined(PLATFORM_3DS)
    return true;
#endif
    return false;
}

void S2S_SetGamePaused(bool paused)
{
    gamePaused = paused;
}

bool S2S_IsGamePaused()
{
    return gamePaused && !consoleGamePaused;
}

void S2S_ClearScreen(Color color)
{
#if defined(PLATFORM_PC)
    glClearColor((float)color.r/255, (float)color.g/255, (float)color.b/255, (float)color.a/255);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#elif defined(PLATFORM_3DS)
    C2D_TargetClear(
        currScreen == TOP ? top : bottom,
        Color_ToUInt32_Default(color));
#endif
}

void SetWindowTitle(const char* new_title) {
#if defined(PLATFORM_PC)
    if (window) {
        SDL_SetWindowTitle(window, new_title);
    }
#endif
    return;
}

bool S2S_ScreensInit()
{
    if(screensInitialized)
        return false;
#if defined(PLATFORM_PC)
    if(!MDS_ACTIVATED)
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
        {
            printf("SDL Init Error: %s\n", SDL_GetError());
            return -1;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

        restoringWindowState = true;
        SaveForCArray *list = loadInitial();

        int32_t saved_w = -1, saved_h = -1;
        int32_t saved_x = -1, saved_y = -1;
        bool has_size = false;
        bool has_pos = false;
        bool fullscreen = false;

        for(unsigned int i = 0; i < list->size; i++)
        {
            if(strcmp(list->list[i].name, "fullscreen") == 0)
            {
                if(list->list[i].type == SAVE_TYPE_BOOL && *(bool*)(list->list[i].data))
                {
                    fullscreen = true;
                }
                continue;
            }
            
            // 1. Leer tamaño guardado (Casteo corregido a int32_t*)
            if(strcmp(list->list[i].name, "window_width") == 0) {
                if(list->list[i].type == SAVE_TYPE_INT32){
                    saved_w = *(int32_t*)(list->list[i].data);
                    has_size = true;
                }
                continue;
            }
            if(strcmp(list->list[i].name, "window_height") == 0) {
                if(list->list[i].type == SAVE_TYPE_INT32){
                    saved_h = *(int32_t*)(list->list[i].data);
                    has_size = true;
                }
                continue;
            }

            // 2. Leer posición guardada (Casteo corregido a int32_t* y bandera corregida a has_pos)
            if(strcmp(list->list[i].name, "window_x") == 0) {
                if(list->list[i].type == SAVE_TYPE_INT32){
                    saved_x = *(int32_t*)(list->list[i].data);
                    has_pos = true; // <- CORREGIDO (Antes era has_size)
                }
                continue;
            }
            if(strcmp(list->list[i].name, "window_y") == 0) {
                if(list->list[i].type == SAVE_TYPE_INT32){
                    saved_y = *(int32_t*)(list->list[i].data);
                    has_pos = true; // <- CORREGIDO (Antes era has_size)
                }
                continue;
            }
        }

        has_size = false;
        has_pos = false;

        printf("[WINDOW] Width: %d, Height: %d.\n", saved_w, saved_h);
        printf("[WINDOW] X: %d, Y: %d.\n", saved_x, saved_y);

        window = SDL_CreateWindow(
                "Game",
                has_pos ? saved_x : SDL_WINDOWPOS_CENTERED,
                has_pos ? saved_y : SDL_WINDOWPOS_CENTERED,
                has_size ? saved_w : wwidth,
                has_size ? saved_h : wheight,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

        gl_context =
            SDL_GL_CreateContext(window);

        if (!gl_context)
        {
            printf("OpenGL Context Error: %s\n", SDL_GetError());
            return -1;
        }

        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
            printf("GLEW error: %s\n", glewGetErrorString(err));
        }
        else
            printf("GLEW OK: %s\n", glewGetString(GLEW_VERSION));

        glViewport(0, 0, wwidth, wheight);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glOrtho(
            0, wwidth,
            wheight, 0,
            -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glEnable(GL_TEXTURE_2D);

        // Activar VSync
        SDL_GL_SetSwapInterval(1);

        PAKL_SetPak("game.pak");

        if(fullscreen)
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

        /*SDL_SetWindowPosition(
            window,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED);

        if(has_size && saved_w > 0 && saved_h > 0) {
            SDL_SetWindowSize(window, saved_w, saved_h);
        }

        SDL_Delay(50);

        if(has_pos && saved_x != -1 && saved_y != -1) {
            SDL_SetWindowPosition(window, saved_x, saved_y);
        }*/

        freeLoadedList(list);
        restoringWindowState = false;
    }

    glEnable(GL_TEXTURE_2D);

#elif defined(PLATFORM_3DS)
    gfxInitDefault();
    if(!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE))
    {
        gfxExit();
        return false;
    }
    top = C2D_CreateScreenTarget(
            GFX_TOP,
            GFX_LEFT);
    bottom = C2D_CreateScreenTarget(
                GFX_BOTTOM,
                GFX_LEFT);
    aptInit();
    srvInit();
    fsInit();
    amInit();
    romfsInit();
    aptHook(&hookCookie, SystemCallback, NULL);
#endif
    screensInitialized = true;
    running = true;

    usedTop = false;
    usedBottom = false;

    currScreen = -1;

    closedCover = false;

    consoleGamePaused = false;

    return true;
}

void S2S_StopRunning()
{
    running = false;
}

bool S2S_ScreensRunning()
{
#if defined(PLATFORM_PC)
    return screensInitialized && running;
#elif defined(PLATFORM_3DS)
    return screensInitialized && running && aptMainLoop();
#endif
}

#if defined(PLATFORM_PC)

typedef enum SCREENS_MODE
{
    TOP_MODE,
    BOTTOM_MODE,
    BOTH_MODE,
} SCREENS_MODE;
static SCREENS_MODE screens_mode = BOTH_MODE;

void TopMode()
{
    float baseTotalWidth = (float)(SCREEN_TOP_WIDTH + (SCREEN_BORDER * 2));
    float baseTotalHeight = (float)(SCREEN_HEIGHT + (SCREEN_BORDER * 2));
    float scaleX = (float)wwidth / baseTotalWidth;
    float scaleY = (float)wheight / baseTotalHeight;
    
    windowScale = min2(scaleX, scaleY);

    float scaledBorder = (float)SCREEN_BORDER * windowScale;

    topWidth = (float)SCREEN_TOP_WIDTH * windowScale;
    topHeight = (float)SCREEN_HEIGHT * windowScale;

    float totalScaledWidth = baseTotalWidth * windowScale;
    float globalXOffset = ((float)wwidth - totalScaledWidth) / 2.0f;

    float totalScaledHeight = baseTotalHeight * windowScale;
    float globalYOffset = ((float)wheight - totalScaledHeight) / 2.0f;
    
    topInitialPointX = globalXOffset + scaledBorder;
    topInitialPointY = globalYOffset + scaledBorder;

    bottomInitialPointX = 0;
    bottomInitialPointY = 0;
    botWidth = 0;
    botHeight = 0;
}
void BottomMode()
{
    float baseTotalWidth = (float)(SCREEN_BOT_WIDTH + (SCREEN_BORDER * 2));
    float baseTotalHeight = (float)(SCREEN_HEIGHT + (SCREEN_BORDER * 2));

    float scaleX = (float)wwidth / baseTotalWidth;
    float scaleY = (float)wheight / baseTotalHeight;

    windowScale = min2(scaleX, scaleY);

    float scaledBorder = (float)SCREEN_BORDER * windowScale;

    botWidth = (float)SCREEN_BOT_WIDTH * windowScale;
    botHeight = (float)SCREEN_HEIGHT * windowScale;

    float totalScaledWidth = baseTotalWidth * windowScale;
    float totalScaledHeight = baseTotalHeight * windowScale;

    float globalXOffset = ((float)wwidth - totalScaledWidth) / 2.0f;
    float globalYOffset = ((float)wheight - totalScaledHeight) / 2.0f;

    bottomInitialPointX = globalXOffset + scaledBorder;
    bottomInitialPointY = globalYOffset + scaledBorder;

    topInitialPointX = 0.0f;
    topInitialPointY = 0.0f;
    topWidth = 0.0f;
    topHeight = 0.0f;
}
void BothMode()
{
    // 1. Incluir el SCREEN_BORDER en las dimensiones base virtuales (en píxeles virtuales)
    // El ancho base es la pantalla TOP + borde izquierdo + borde derecho
    float baseTotalWidth = (float)(SCREEN_TOP_WIDTH + (SCREEN_BORDER * 2));
    // El alto base son ambas pantallas + el GAP + borde superior + borde inferior
    float baseTotalHeight = (float)(SCREEN_HEIGHT * 2 + SCREEN_GAP + (SCREEN_BORDER * 2));

    // Calcular escala basándose en el tamaño total con bordes incluidos
    float scaleX = (float)wwidth / baseTotalWidth;
    float scaleY = (float)wheight / baseTotalHeight;
    
    windowScale = min2(scaleX, scaleY);

    // 2. Tamaños de pantalla finales escalados
    topWidth  = (float)SCREEN_TOP_WIDTH * windowScale;
    topHeight = (float)SCREEN_HEIGHT * windowScale;
    botWidth  = (float)SCREEN_BOT_WIDTH * windowScale;
    botHeight = (float)SCREEN_HEIGHT * windowScale;
    float scaledGap = (float)SCREEN_GAP * windowScale;
    
    // El tamaño del borde también se escala proporcionalmente
    float scaledBorder = (float)SCREEN_BORDER * windowScale;

    // 3. Centrado del bloque completo en el eje X
    // Calculamos dónde empieza el bloque total y le sumamos el borde escalado para las pantallas
    float totalScaledWidth = baseTotalWidth * windowScale;
    float globalXOffset = ((float)wwidth - totalScaledWidth) / 2.0f;
    
    // Posición X de las pantallas sumando el borde lateral
    topInitialPointX = globalXOffset + scaledBorder;
    // La pantalla de abajo se centra respecto a la de arriba (o mantiene el desfase si tuviera otra lógica)
    bottomInitialPointX = globalXOffset + scaledBorder + (((float)(SCREEN_TOP_WIDTH - SCREEN_BOT_WIDTH) * windowScale) / 2.0f);

    // 4. Centrado del bloque completo en el eje Y
    float totalScaledHeight = baseTotalHeight * windowScale;
    float globalYOffset = ((float)wheight - totalScaledHeight) / 2.0f;

    // La pantalla TOP empieza después del offset global del contenedor + el borde superior escalado
    topInitialPointY = globalYOffset + scaledBorder;

    // La pantalla BOTTOM se posiciona sumando TOP y el gap
    bottomInitialPointY = topInitialPointY + topHeight + scaledGap;
}

void CycleScreenMode(void)
{
    switch (screens_mode)
    {
        case BOTH_MODE:
            screens_mode = TOP_MODE;
            TopMode();
            break;

        case TOP_MODE:
            screens_mode = BOTTOM_MODE;
            BottomMode();
            break;

        case BOTTOM_MODE:
        default:
            screens_mode = BOTH_MODE;
            BothMode();
            break;
    }
}

void S2S_UpdateWindowScreensi()
{
    switch (screens_mode)
    {
    case TOP_MODE:
        TopMode();
        break;
    case BOTTOM_MODE:
        BottomMode();
        break;
    case BOTH_MODE:
        BothMode();
        break;
    
    default:
        BothMode();
        break;
    }
}

bool isFullscreen = false;
#endif

void S2S_BeginFrame()
{

#if defined(PLATFORM_PC)
    if(MDS_ACTIVATED)
        return;
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if(!restoringWindowState)
        {
            if (event.type == SDL_QUIT)
                running = false;
            else if (event.type == SDL_WINDOWEVENT) {
                // Capturar eventos de la ventana (Resize y Move)
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED: {
                        int32_t *width = malloc(sizeof(int32_t));
                        int32_t *height = malloc(sizeof(int32_t));
                        *width = event.window.data1;
                        *height = event.window.data2;
                        printf("[WINDOW] Width: %d, Height: %d.\n", *width, *height);

                        setScreenValue("window_width", width, SAVE_TYPE_INT32);
                        setScreenValue("window_height", height, SAVE_TYPE_INT32);

                        free(width);
                        free(height);
                        break;
                    }
                    case SDL_WINDOWEVENT_MOVED: {
                        int32_t *posX = malloc(sizeof(int32_t));
                        int32_t *posY = malloc(sizeof(int32_t));
                        *posX = event.window.data1;
                        *posY = event.window.data2;

                        setScreenValue("window_x", posX, SAVE_TYPE_INT32);
                        setScreenValue("window_y", posY, SAVE_TYPE_INT32);

                        free(posX);
                        free(posY);
                        break;
                    }
                }
            }
            else if (event.type == SDL_KEYDOWN) {
                // Evita repetir mientras la tecla está mantenida
                if (!event.key.repeat)
                {
                    SDL_Keymod mods = SDL_GetModState();

                    if ((mods & KMOD_CTRL) &&
                        event.key.keysym.sym == SDLK_TAB)
                    {
                        CycleScreenMode();
                    }
                }

                switch (event.key.keysym.sym) {
                    // 2. Escuchar la tecla F11
                    case SDLK_F11:
                        isFullscreen = !isFullscreen; // Alternar estado

                        bool *fs = malloc(sizeof(bool));
                        *fs = isFullscreen;
                        
                        // 3. Cambiar el modo de la ventana de forma segura
                        if (isFullscreen) {
                            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                            setScreenValue("fullscreen", fs, SAVE_TYPE_BOOL);
                        } else {
                            SDL_SetWindowFullscreen(window, 0); // Vuelve a modo ventana
                            setScreenValue("fullscreen", fs, SAVE_TYPE_BOOL);
                        }
                        free(fs);

                        break;
                }
            }
        }
    }

    SDL_GetWindowSize(window, &wwidth, &wheight);
    glViewport(0, 0, wwidth, wheight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(
        0, wwidth,
        wheight, 0,
        -1, 1);

    S2S_UpdateWindowScreensi();
    
    // 1. Limpiar el fondo general de la ventana (Fondo gris/blanco exterior)
    S2S_ClearScreen(Color_MakeColor(25,25,25,255)); 

    glEnable(GL_SCISSOR_TEST);

    // 2. DIBUJAR EL BORDE CONTENEDOR (SCREEN_BORDER)
    // Calculamos el área total que ocupan las pantallas juntas incluyendo sus bordes externos
    float scaledBorder = (float)SCREEN_BORDER * windowScale;
    float containerX = topInitialPointX - scaledBorder;
    float containerY = topInitialPointY - scaledBorder;
    float containerWidth = (float)SCREEN_TOP_WIDTH * windowScale + (scaledBorder * 2);
    float containerHeight = (float)(SCREEN_HEIGHT * 2 + SCREEN_GAP) * windowScale + (scaledBorder * 2);
    
    // Invertir Y para el sistema de coordenadas de OpenGL Scissor
    int scissorContainerY = wheight - (int)(containerY + containerHeight);
    glScissor((int)containerX, scissorContainerY, (int)containerWidth, (int)containerHeight);
    
    // Color del borde (por ejemplo, Gris Oscuro o Negro para simular el plástico de la consola)
    //S2S_ClearScreen(Color_MakeColor(40, 40, 40, 255)); 

    // 3. Pantalla TOP
    int scissorTopY = wheight - (int)(topInitialPointY + topHeight);
    glScissor((int)topInitialPointX, scissorTopY, (int)topWidth, (int)topHeight);
    S2S_ClearScreen(Color_Black);

    // 4. Pantalla BOTTOM
    int scissorBotY = wheight - (int)(bottomInitialPointY + botHeight);
    glScissor((int)bottomInitialPointX, scissorBotY, (int)botWidth, (int)botHeight);
    S2S_ClearScreen(Color_Black);
#elif defined(PLATFORM_3DS)
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    currScreen = TOP;
    S2S_ClearScreen(Color_Black);
    currScreen = BOTTOM;
    S2S_ClearScreen(Color_Black);
#endif

    D2D_TextsBegin();
    currScreen = -1;
}

void S2S_EndFrame()
{
#if defined(PLATFORM_PC)
    if(!MDS_ACTIVATED)
    {
        glDisable(GL_SCISSOR_TEST);
        // Presentar frame OpenGL
        SDL_GL_SwapWindow(window);
    }
#elif defined(PLATFORM_3DS)
    C3D_FrameEnd(0);
#endif
    usedTop = false;
    usedBottom = false;
    currScreen = -1;
    D2D_TextsEnd();
}

void S2S_SetCurrentScreen(S2S_Screen screen)
{
    if(!S2S_ScreensRunning())
        return;

    if(screen != TOP && screen != BOTTOM)
        return;
        
    if(screen == TOP ? (usedTop) : (usedBottom))
    {
        currScreen = -1;
        return;
    }
    currScreen = screen;
    screen == TOP ? (usedTop = true) : (usedBottom = true);
#if defined(PLATFORM_PC)
    if(!MDS_ACTIVATED)
    {
        if(screen == TOP)
        {
            int scissorY = wheight - (int)(topInitialPointY + topHeight);
            glScissor((int)topInitialPointX, scissorY, (int)topWidth, (int)topHeight);
        }
        else
        {
            int scissorY = wheight - (int)(bottomInitialPointY + botHeight);
            glScissor((int)bottomInitialPointX, scissorY, (int)botWidth, (int)botHeight);
        }
    }
#elif defined(PLATFORM_3DS)
    C2D_SceneTarget(currScreen == TOP ? top : bottom);
    C2D_SceneBegin(currScreen == TOP ? top : bottom);
#endif

    t3da_set_screen_size(currScreen == TOP ? SCREEN_TOP_WIDTH : SCREEN_BOT_WIDTH, SCREEN_HEIGHT);
}

Vec2 S2S_GetScreenSize(S2S_Screen screen)
{
    if(screen == TOP)
        return vec2_create(SCREEN_TOP_WIDTH, SCREEN_HEIGHT);
    else if (screen == BOTTOM)
        return vec2_create(SCREEN_BOT_WIDTH, SCREEN_HEIGHT);
    else 
        return vec2_create(0, 0);
}

void S2S_ScreensExit()
{
    if(!screensInitialized)
        return;
#if defined(PLATFORM_PC)
    PAKL_ClosePak();
    if(!MDS_ACTIVATED)
    {
        S2S_ClearScreen(Color_Black);

        SDL_GL_DeleteContext(gl_context);

        SDL_DestroyWindow(window);

        SDL_Quit();
    }
    else
        MDS_ACTIVATED = false;
#elif defined(PLATFORM_3DS)
    aptUnhook(&hookCookie);
    amExit();
    fsExit();
    srvExit();
    aptExit();
    C3D_Fini();
    romfsExit();
    gfxExit();
#endif
    screensInitialized = false;
    running = false;
    currScreen = -1;
}


void setDrawRegion(float x, float y, float w, float h)
{
    float ss = currScreen == TOP ? SCREEN_TOP_WIDTH : SCREEN_BOT_WIDTH;
    x = clampf(x, 0, ss);
    y = clampf(y, 0, SCREEN_HEIGHT);
    w = clampf(w, 0, ss-x);
    h = clampf(h, 0, SCREEN_HEIGHT-y);
    if(currScreen == TOP)
    {
#if defined(PLATFORM_PC)
        glScissor(
            (int)(topInitialPointX + x * windowScale),
            wheight - (int)(topInitialPointY + (y + h) * windowScale),
            (int)(w * windowScale),
            (int)(h * windowScale)
        );
#elif defined(PLATFORM_3DS)
        C3D_SetScissor(GPU_SCISSOR_NORMAL, x, y, x+w, y+h);
#endif
    }
    else if(currScreen == BOTTOM)
    {
#if defined(PLATFORM_PC)
        glScissor(
            (int)(bottomInitialPointX + x * windowScale),
            wheight - (int)(bottomInitialPointY + (y + h) * windowScale),
            (int)(w * windowScale),
            (int)(h * windowScale)
        );
#elif defined(PLATFORM_3DS)
        C3D_SetScissor(GPU_SCISSOR_NORMAL, x, SCREEN_HEIGHT-(y+h), x+w, SCREEN_HEIGHT-y);
#endif
    }
}

void stopDrawRegion()
{
    if(currScreen == TOP)
    {
#if defined(PLATFORM_PC)
        int scissorTopY = wheight - (int)(topInitialPointY + topHeight);
        glScissor((int)topInitialPointX, scissorTopY, (int)topWidth, (int)topHeight);
#elif defined(PLATFORM_3DS)
        C3D_FrameDrawOn(top);
        C2D_Flush();
        C3D_SetScissor(GPU_SCISSOR_DISABLE, 0, 0, 0, 0);
#endif
    }
    else if(currScreen == BOTTOM)
    {
#if defined(PLATFORM_PC)
        int scissorBotY = wheight - (int)(bottomInitialPointY + botHeight);
        glScissor((int)bottomInitialPointX, scissorBotY, (int)botWidth, (int)botHeight);
#elif defined(PLATFORM_3DS)
        C3D_FrameDrawOn(bottom);
        C2D_Flush();
        C3D_SetScissor(GPU_SCISSOR_DISABLE, 0, 0, 0, 0);
#endif
    }
}