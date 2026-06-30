#pragma once

#include <screens.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

// Si este .c define ALLOCATE_SHMEM, se crean las variables.
// Si no lo define, se tratan como extern automáticos.
#ifdef ALLOCATE_SHMEM
  #define SHMEM_EXT
  #define SHMEM_INIT(x) = x
#else
  #define SHMEM_EXT extern
  #define SHMEM_INIT(x)
#endif

SHMEM_EXT S2S_Screen currScreen SHMEM_INIT(TOP);
SHMEM_EXT bool usedTop SHMEM_INIT(false);
SHMEM_EXT bool usedBottom SHMEM_INIT(false);
SHMEM_EXT bool screensInitialized SHMEM_INIT(false);
SHMEM_EXT bool gamePaused SHMEM_INIT(false);

#if defined(PLATFORM_PC)
#include <SDL2/SDL.h>
#include <GL/glew.h>

SHMEM_EXT int wwidth SHMEM_INIT(1280);
SHMEM_EXT int wheight SHMEM_INIT(720);
SHMEM_EXT SDL_Window *window SHMEM_INIT(NULL);
SHMEM_EXT SDL_GLContext gl_context SHMEM_INIT(NULL);
SHMEM_EXT float topInitialPointX SHMEM_INIT(0.0f);
SHMEM_EXT float topInitialPointY SHMEM_INIT(0.0f);
SHMEM_EXT float bottomInitialPointX SHMEM_INIT(0.0f);
SHMEM_EXT float bottomInitialPointY SHMEM_INIT(0.0f);
SHMEM_EXT float topWidth SHMEM_INIT(0.0f);
SHMEM_EXT float topHeight SHMEM_INIT(0.0f);
SHMEM_EXT float botWidth SHMEM_INIT(0.0f);
SHMEM_EXT float botHeight SHMEM_INIT(0.0f);
SHMEM_EXT float windowScale SHMEM_INIT(1.0f);
#endif

void setDrawRegion(float x, float y, float w, float h);
void stopDrawRegion();

// Convertimos la función en 'static inline' para que cada .c 
// tenga su copia de la lógica sin pelearse en el enlazador.
static inline bool isValidScreen(void)
{
    if(currScreen != TOP && currScreen != BOTTOM)
        return false;
    return true;
}

#ifdef __cplusplus
}
#endif
