#include <sys/system_fonts.h>

#include <string>
#include <vector>
#include <cstdint>

#include "draw/2d/2d_vals.h"

#include "sys_fonts_gen_funcs.h"

std::vector<D2D_Font*> fonts = {};

size_t defaultFontIndex = 0;

std::vector<std::string> fontsNames = {
    "arial"
};

void CloseAllFonts()
{
    for(auto &f : fonts)
    {
        D2D_CloseFont_Buf(f, false);
    }
    fonts = {};
}

void InitAllFonts()
{
    CloseAllFonts();
    for(auto &name : fontsNames)
    {
        std::string path = "engine/fonts/" + name;

        D2D_Font *f = D2D_OpenFont_Buf(path.c_str(), false);

        fonts.push_back(f);

        printf("Opened font: %s\n", name.c_str());

        if (f == nullptr) {
            printf("[ERROR] No se pudo cargar: %s (Ruta intentada: %s)\n", name.c_str(), path.c_str());
        } else {
            printf("[OK] Fuente cargada correctamente: %s (Puntero: %p)\n", name.c_str(), (void*)f);
        }
    }
}

const char* System_GetDefaultFontName()
{
    if(fonts.size() <= 0)
        return "";

    return fontsNames[defaultFontIndex].c_str();
}

D2D_Font *System_GetDefaultFont()
{
    if(fonts.size() <= 0)
        return nullptr;

    D2D_Font *f = fonts[defaultFontIndex];

    return f;
}

size_t System_GetFontsSize()
{
    return fonts.size();
}

const char *System_GetFontNameByIndex(size_t index)
{
    if(index >= fonts.size())
        return "";

    return fontsNames[index].c_str();
}

D2D_Font *System_GetFontByIndex(size_t index)
{
    if(index >= fonts.size())
        return nullptr;
    return fonts[index];
}