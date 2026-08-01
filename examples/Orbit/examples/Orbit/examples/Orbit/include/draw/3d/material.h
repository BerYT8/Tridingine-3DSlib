#pragma once

#include <color.h>
#include <textures/t3da_types.h>

typedef struct Material
{
    Color color;
    T3DA_AtlasTexture *diffusion;
    T3DA_AtlasTexture *normal;
    T3DA_AtlasTexture *mat;
} Material;
