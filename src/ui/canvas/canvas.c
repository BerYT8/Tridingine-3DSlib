#include <ui/canvas/canvas.h>
#include "../definitions.h"
#include <stdlib.h>

UI_Canva UI_CreateCanva()
{
    UI_Canva c = malloc(sizeof(UI_Element_s));
    if(!c)
        return NULL;
    c->type = TYPE_CANVA;

    c->color = Color_MakeColor(255, 255, 255, 0);

    c->startingPosX = 0;
    c->startingPosY = 0;

    c->alignX = 0;
    c->alignY = 0;
    c->screenAlignX = 0;
    c->screenAlignY = 0;

    c->posX = 0;
    c->posY = 0;

    c->width = 0;
    c->height = 0;

    c->assigned = NULL;

    return c;
}