#pragma once

#include <color.h>

typedef struct UI_Element_s UI_Element_s;

typedef enum UI_Element_Type
{
    TYPE_CANVA,
    TYPE_LABEL,
    TYPE_BUTTON,
} UI_Element_Type;

typedef struct UI_Element_s
{
    float screenAlignX, screenAlignY;
    float alignX, alignY;
    float width, height;
    float posX, posY;
    float startingPosX, startingPosY;

    Color color;

    UI_Element_Type type;

    UI_Element_s *assigned;
} UI_Element_s;
