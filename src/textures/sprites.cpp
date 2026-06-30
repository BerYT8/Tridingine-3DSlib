#if defined(PLATFORM_3DS)
#include "sprites.h"

Tex3DS_SubTexture *fix_UV(C2D_Image *image)
{
    C3D_Tex *tex = image->tex;
    Tex3DS_SubTexture *subTex = new Tex3DS_SubTexture();

    float EPSILON = 0.00001f;

    subTex->left = image->subtex->left + EPSILON;
    subTex->right = image->subtex->right - EPSILON;
    subTex->top = image->subtex->top - EPSILON;
    subTex->bottom = image->subtex->bottom + EPSILON;

    subTex->width = image->subtex->width;
    subTex->height = image->subtex->height;

    C2D_Image img = { tex, subTex };
    *image = img;
    return subTex;
}
#endif