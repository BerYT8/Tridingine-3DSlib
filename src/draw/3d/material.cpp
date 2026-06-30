#include <draw/3d/material.h>

#include "../../textures/textures_types.h"

static C3D_Tex gWhiteTex;

bool MAT3D_Init()
{
    C3D_TexInit(&gWhiteTex, 1, 1, GPU_RGBA8);
    C3D_TexSetFilter(&gWhiteTex, GPU_NEAREST, GPU_NEAREST);
    C3D_TexSetWrap(&gWhiteTex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);

    // Escribir el único píxel
    u32* data = (u32*)gWhiteTex.data;
    data[0] = 0xFFFFFFFF; // Blanco RGBA

    return true;
}

bool MAT3D_CreateMaterial(Material *mat, const char* path)
{
    Material material;

    material.diffusion = nullptr;
    material.color = Color_White;
    material.normal = nullptr;
    material.mat = nullptr;

    // Código 

    *mat = material;

    return false;
}

void MAT3D_BindMaterial(Material material)
{
    C3D_Tex* tex =
        material.diffusion ?
        C2D_SpriteSheetGetImage(material.diffusion->sheet, 0).tex :
        &gWhiteTex;

    if (!tex)
        tex = &gWhiteTex;

    C3D_TexBind(0, tex);

    C3D_TexEnv* env0 = C3D_GetTexEnv(0);

    C3D_TexEnvSrc(env0, C3D_Both,
                GPU_TEXTURE0,
                GPU_PRIMARY_COLOR,
                GPU_CONSTANT);

    C3D_TexEnvFunc(env0, C3D_Both, GPU_MODULATE);

    C3D_ImmSendAttrib(
        material.color.r,
        material.color.g,
        material.color.b,
        material.color.a
    );

}

void MAT3D_Exit()
{
    C3D_TexDelete(&gWhiteTex);
}