#include <modify_screens.h>
#include "screensValues.h"

void MDS_Activate()
{
#if defined(PLATFORM_PC)
    if(screensInitialized)
        return;
    MDS_ACTIVATED = true;
    windowScale = 1;
#endif
}

void MDS_SetWindowSize(int w, int h)
{
#if defined(PLATFORM_PC)
    if(MDS_ACTIVATED)
    {
        wwidth = w;
        wheight = h;
    }
#endif
}

void MDS_SetWindowScale(float scale)
{
#if defined(PLATFORM_PC)
    if(MDS_ACTIVATED)
    {
        windowScale = scale;
    }
#endif
}

void MDS_SetTopScreen(float x, float y, float w, float h)
{
#if defined(PLATFORM_PC)
    if(MDS_ACTIVATED)
    {
        topInitialPointX = x;
        topInitialPointY = y;
        topWidth = w;
        topHeight = h;
    }
#endif
}

void MDS_SetBottomScreen(float x, float y, float w, float h)
{
#if defined(PLATFORM_PC)
    if(MDS_ACTIVATED)
    {
        bottomInitialPointX = x;
        bottomInitialPointY = y;
        botWidth = w;
        botHeight = h;
    }
#endif
}