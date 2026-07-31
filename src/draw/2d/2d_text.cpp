#include <draw/2d/2d_shapes.h>

#include "2d_vals.h"

#include <maths.h>

#if defined(PLATFORM_PC)
#include <SDL.h>
#include <SDL_ttf.h>
#elif defined(PLATFORM_3DS)
#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#endif
#include "../../screens/screensValues.h"
#include <vector>
#include <string>
#include <sstream>
#include <pak_loader/pak_loader.h>

typedef struct D2D_Font
{
#if defined(PLATFORM_PC)
    TTF_Font *font;
#elif defined(PLATFORM_3DS)
#endif
} D2D_Font;

#if defined(PLATFORM_PC)
#endif

D2D_Font *D2D_OpenFont(const char* path)
{
    D2D_Font *ft = new D2D_Font();
    if(!ft)
        return nullptr;

#if defined(PLATFORM_PC)
    PAK_FILE* f = PAKL_LoadFile(path);
    if (!f)
    {
        delete ft;
        return nullptr;
    } 

    PAKL_fseek(f, 0, SEEK_END);
    long tamano = PAKL_ftell(f);
    PAKL_rewind(f);
    
    if (tamano <= 0) {
        PAKL_CloseFile(f);
        delete ft;
        return nullptr;
    }

    void* buffer = malloc(tamano);
    if (!buffer) {
        PAKL_CloseFile(f);
        delete ft;
        return nullptr;
    }

    size_t leidos = PAKL_fread(buffer, 1, tamano, f);
    PAKL_CloseFile(f);

    SDL_RWops* rw = nullptr;

    if (leidos == (size_t)tamano)
        rw = SDL_RWFromMem((void*)buffer, tamano);

    if (!rw) {
        SDL_Log("Error al crear RWops: %s", SDL_GetError());
        free(buffer);
        delete ft;
        return nullptr;
    }
    ft->font = TTF_OpenFontRW(rw, 1, 24);
    free(buffer);
#elif defined(PLATFORM_3DS)
#endif
    return ft;
}

void D2D_CloseFont(D2D_Font *font)
{
    if(!font)
        return;
#if defined(PLATFORM_PC)
    TTF_CloseFont(font->font);
    delete font;
#elif defined(PLATFORM_3DS)
#endif
}

D2D_Result D2D_DrawText(
    const char* text,
    D2D_Font* font,
    float fontSize,
    Color color,

    float x,
    float y,
    float depth,
    float w,
    float h,

    float alignX,
    float alignY,

    float letterSpacing,
    float lineSpacing,

    D2D_WrapMode wrap
)
{
    if(!initialized)
        return D2D_NOT_INITIALIZED;

    if(depth < -1 || depth > 1 || !font)
        return D2D_INVALID_ARGUMENT;

    if(!isValidScreen())
        return D2D_ERROR;

    printf("[D2D_DrawText] ENTER\n");
    printf("text: %s\n", text ? text : "NULL");
    printf("font: %p\n", font);
    printf("fontSize: %f\n", fontSize);
    printf("pos: (%f, %f)\n", x, y);
    printf("size: (%f, %f)\n", w, h);

#if defined(PLATFORM_PC)
    w *= windowScale;
    h *= windowScale;
    x *= windowScale;
    y *= windowScale;

    x += (currScreen == TOP ? topInitialPointX : bottomInitialPointX);
    y += (currScreen == TOP ? topInitialPointY : bottomInitialPointY);
#endif

    alignX = clampf(alignX, 0.f, 1.f);
    alignY = clampf(alignY, 0.f, 1.f);

    if(fontSize < 0)
        fontSize = 0;
    if(w < 0)
        w = 0;
    if(h < 0)
        h = 0;

#if defined(PLATFORM_PC)

    std::vector<SDL_Surface*> surfs = {};
    SDL_Color c;
    c.r = color.r;
    c.g = color.g;
    c.b = color.b;
    c.a = color.a;
    TTF_SetFontSize(font->font, fontSize*windowScale);

    int height = TTF_FontHeight(font->font);

    printf("[D2D_DrawText] splitting words...\n");

    std::vector<std::string> words = {};
    std::istringstream iss(text);

    std::string word;
    while (iss >> word) {
        printf("word: %s\n", word.c_str());
        words.push_back(word);
    }

    std::vector<Vec2> linesSize;
    std::vector<std::string> lines;

    switch (wrap)
    {
    case WORD_WRAP_MODE:
    {
        printf("\n[D2D_DrawText] WORD_WRAP_MODE start\n");

        size_t offset = 0;

        while (offset < words.size())
        {
            printf("offset: %zu\n", offset);

            std::string line = "";
            size_t count = 0;

            int tw = 0, th = 0;
            int lastValidW = 0, lastValidH = 0;

            while (offset + count < words.size())
            {
                std::string testLine;

                // construir línea candidata
                for (size_t i = 0; i <= count; i++)
                {
                    if (i > 0) testLine += " ";
                    testLine += words[offset + i];
                }

                printf("testLine: %s\n", testLine.c_str());

                TTF_SizeText(font->font, testLine.c_str(), &tw, &th);

                // si se pasa, paramos ANTES de incluir esta palabra
                if (tw > w)
                {
                    printf("wrap break BEFORE word: tw=%d w=%f\n", tw, w);
                    break;
                }

                // aceptar palabra
                line = testLine;
                lastValidW = tw;
                lastValidH = th;
                count++;
            }

            // si no se pudo añadir ninguna palabra, forzar 1 palabra
            if (count == 0)
            {
                printf("FORCED SINGLE WORD LINE: %s\n", words[offset].c_str());

                line = words[offset];
                TTF_SizeText(font->font, line.c_str(), &lastValidW, &lastValidH);

                count = 1;
            }

            printf("final line: %s\n", line.c_str());

            lines.push_back(line);
            linesSize.push_back(vec2_create(lastValidW, lastValidH));

            offset += count;
        }

        break;
    }
    case LETTER_WRAP_MODE:
        /* code */
        break;
    case WRAP_NONE:
    default:
        break;
    }

    words.clear();

    if(lines.size() < 1)
        return D2D_ERROR;

    for(size_t i = 0; i < lines.size(); i++)
    {
        surfs.push_back(TTF_RenderText_Solid(font->font, lines[i].c_str(), c));
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(
        0,
        wwidth,
        wheight,
        0,
        -1,
        1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();

    glUseProgram(0);

    for (size_t i = 0; i < surfs.size(); i++)
    {
        SDL_Surface* conv = SDL_ConvertSurfaceFormat(
            surfs[i],
            SDL_PIXELFORMAT_RGBA32,
            0
        );
        SDL_FreeSurface(surfs[i]);

        GLuint texture;
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
             conv->w, conv->h,
             0, GL_RGBA, GL_UNSIGNED_BYTE,
             conv->pixels);
        printf("%x\n", glGetError());

        // Coordenadas destino
        float x1 = x - w*alignX + (w-linesSize[i].x);
        float y1 = y - h*alignY + (h-linesSize[i].y);
        float x2 = x1 + linesSize[i].x;
        float y2 = y1 + linesSize[i].y;

        //setDrawRegion(x1, y1, w, h);
        // Render quad
        glBindTexture(GL_TEXTURE_2D, texture);

        glColor4ub(255, 255, 255, 255);

        glBegin(GL_QUADS);
            glTexCoord2f(0, 0); glVertex2f(x1, y1);
            glTexCoord2f(1, 0); glVertex2f(x2, y1);
            glTexCoord2f(1, 1); glVertex2f(x2, y2);
            glTexCoord2f(0, 1); glVertex2f(x1, y2);
        glEnd();

        //stopDrawRegion();
        
        glBindTexture(GL_TEXTURE_2D, 0);

        // avanzar en Y
        y += height + lineSpacing;

        glDeleteTextures(1, &texture);
        SDL_FreeSurface(conv);
    }

    glPopMatrix();

    surfs.clear();

#elif defined(PLATFORM_3DS)
#endif

    return D2D_OK;
}