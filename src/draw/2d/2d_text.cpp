#include <draw/2d/2d_shapes.h>

#include "2d_vals.h"

#include <maths.h>

#ifndef INITIAL_FONT_SIZE
#define INITIAL_FONT_SIZE 24
#endif

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
#include "../../romfs_path.h"
#include "../../console/drawConsole.h"
#include "../../sys_fonts_gen_funcs.h"

static bool textsInitialized = false;

#ifndef MAX_CHARACTERS_FT
#define MAX_CHARACTERS_FT 4096
#endif
u32 currentCharactersCount = 0;

#if defined(PLATFORM_3DS)
C2D_TextBuf globalBuffer = nullptr;
#endif

D2D_Font *D2D_OpenFont_Buf(const char* path, bool del)
{
    D2D_Font *ft = new D2D_Font();
    if(!ft)
        return nullptr;
#if defined(PLATFORM_PC)
    PAK_FILE* f = PAKL_LoadFile((std::string(path) + ".ttf").c_str());
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
    ft->font = TTF_OpenFontRW(rw, 1, INITIAL_FONT_SIZE);
    if(!ft->font)
    {
        delete ft;
        free(buffer);
        return nullptr;
    }
    ft->buffer = buffer;
#elif defined(PLATFORM_3DS)
    std::string full = getRomfsPath((std::string(path) + ".bcfnt").c_str());
    printf("%s\n", full.c_str());
    ft->font = C2D_FontLoad(full.c_str());
    
    if(!ft->font)
    {
        delete ft;
        return nullptr;
    }
#endif
    ft->deletable = del;
    return ft;
}

D2D_Font *D2D_OpenFont(const char* path)
{
    return D2D_OpenFont_Buf(path, true);
}

void D2D_CloseFont_Buf(D2D_Font *font, bool del)
{
    if(!font)
        return;
    if(!font->deletable && del)
        return;
#if defined(PLATFORM_PC)
    TTF_CloseFont(font->font);
    free(font->buffer);
#elif defined(PLATFORM_3DS)
    C2D_FontFree(font->font);
#endif
    delete font;
}

void D2D_CloseFont(D2D_Font *font)
{
    D2D_CloseFont_Buf(font, true);
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

    float textAlignX,
    float textAlignY,

    float letterSpacing,
    float lineSpacing,

    D2D_WrapMode wrap
)
{
    if(!initialized || !textsInitialized)
        return D2D_NOT_INITIALIZED;

    if(!isValidScreen())
        return D2D_ERROR;

    if(currentCharactersCount >= MAX_CHARACTERS_FT - 3)
        return D2D_ERROR;

    auto res = D2D_DrawText_Buf(text, font, fontSize, color, x, y, depth, w, h, alignX, alignY, textAlignX, textAlignY, letterSpacing, lineSpacing, wrap, false, TOP_CONSOLE);
    return res.drawed ? D2D_OK : D2D_ERROR;
}

void InitConsoleBuffs()
{
#if defined(PLATFORM_3DS)
    consoleTopBuffer = C2D_TextBufNew(MAX_CHARACTERS_FT);
    consoleBotBuffer = C2D_TextBufNew(MAX_CHARACTERS_FT);
#endif
}
void EndConsoleBuffs()
{
#if defined(PLATFORM_3DS)
    if(consoleTopBuffer)
        C2D_TextBufDelete(consoleTopBuffer);
    if(consoleBotBuffer)
        C2D_TextBufDelete(consoleBotBuffer);
    consoleTopBuffer = nullptr;
    consoleBotBuffer = nullptr;
#endif
}

void ClearConsoleBuf(ScreenConsole console)
{
    if(console != TOP_CONSOLE && console != BOTTOM_CONSOLE)
        return;
#if defined(PLATFORM_3DS)
    auto buf = console == TOP_CONSOLE ? consoleTopBuffer : consoleBotBuffer;
    if(buf)
        C2D_TextBufClear(buf);
    buf = nullptr;
#endif
}

D2D_Text D2D_DrawText_Buf(
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

    float textAlignX,
    float textAlignY,

    float letterSpacing,
    float lineSpacing,

    D2D_WrapMode wrap,
    bool console,
    ScreenConsole consoleN
)
{
    D2D_Text textResult;
    textResult.drawed = false;
    textResult.height = 0;
    textResult.width = 0;

    if(depth < -1 || depth > 1 || !font)
        return textResult;

    if(!font->font)
        return textResult;
        

    std::string textString = text;
    if(textString.length() >= MAX_CHARACTERS_FT - currentCharactersCount)
    {
        textString.erase(MAX_CHARACTERS_FT - 3); // Dejas espacio para los 3 puntos
        textString += "...";
    }

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

    bool autoFontSize = (fontSize < 0.0f);

    if(!autoFontSize && fontSize < 0)
        fontSize = 0;
    if(w < 0)
        w = 0;
    if(h < 0)
        h = 0;

    // Posición del campo según align
    float fieldX = x - w * alignX;
    float fieldY = y - h * alignY;

    std::vector<std::string> lines;
    std::vector<Vec2> linesSize;

    float lineHeight = 0.0f;

#if defined(PLATFORM_3DS)
    C2D_TextBuf textBuffer = console ? (consoleN == TOP_CONSOLE ? consoleTopBuffer : consoleBotBuffer) : globalBuffer;

    if(!textBuffer)
        return textResult;
#endif

    float currentFontSize = fontSize;

    auto MeasureText = [&](const std::string& str, float& tw, float& th)
    {
#if defined(PLATFORM_PC)

        int w = 0;
        int h = 0;

        TTF_SizeText(font->font, str.c_str(), &w, &h);

        tw = (float)w;
        th = (float)h;

#elif defined(PLATFORM_3DS)
        C2D_Text txt;

        C2D_TextParse(&txt, textBuffer, str.c_str());
        C2D_TextOptimize(&txt);

        float scale = currentFontSize / INITIAL_FONT_SIZE;

        if(scale <= 0.0f)
            scale = 1.0f;

        C2D_TextGetDimensions(
            &txt,
            scale,
            scale,
            &tw,
            &th
        );

#endif
    };

#if defined(PLATFORM_PC)

    SDL_Color c;
    c.r = color.r;
    c.g = color.g;
    c.b = color.b;
    c.a = color.a;

    TTF_SetFontSize(font->font, fontSize * windowScale);

    lineHeight = TTF_FontHeight(font->font);

#elif defined(PLATFORM_3DS)

    lineHeight = INITIAL_FONT_SIZE * (fontSize / INITIAL_FONT_SIZE);

#endif

    auto PushLine = [&](const std::string& str)
    {
        float tw;
        float th;

        MeasureText(
            str,
            tw,
            th
        );

        lines.push_back(str);
        linesSize.push_back(
            vec2_create(
                tw,
                th
            )
        );
    };

    float totalHeight = 0;
    float drawY = 0;

    auto CalculateTextSize = [&]() -> bool
    {
        #if defined(PLATFORM_3DS)
        if(textBuffer)
            C2D_TextBufClear(textBuffer);
        #endif
        lines.clear();
        linesSize.clear();
        totalHeight = 0.0f;

        if(wrap == WRAP_NONE)
        {
            std::stringstream ss(textString);
            std::string line;

            while(std::getline(ss, line, '\n'))
                PushLine(line);
        }
        else if(wrap == WORD_WRAP_MODE)
        {
            std::stringstream ss(textString);

            std::string paragraph;

            while(std::getline(ss, paragraph, '\n'))
            {
                std::istringstream iss(paragraph);

                std::string word;
                std::string current;

                while(iss >> word)
                {
                    std::string test =
                        current.empty()
                            ? word
                            : current + " " + word;

                    float tw;
                    float th;

                    MeasureText(
                        test,
                        tw,
                        th
                    );

                    if(tw <= w || current.empty())
                    {
                        current = test;
                    }
                    else
                    {
                        PushLine(current);
                        current = word;
                    }
                }

                PushLine(current);
            }
        }
        else
        {
            std::stringstream ss(textString);

            std::string paragraph;

            while(std::getline(ss, paragraph, '\n'))
            {
                std::string current;

                for(char ch : paragraph)
                {
                    std::string test = current;
                    test += ch;

                    float tw;
                    float th;

                    MeasureText(
                        test,
                        tw,
                        th
                    );

                    if(tw <= w || current.empty())
                    {
                        current = test;
                    }
                    else
                    {
                        PushLine(current);
                        current.clear();
                        current += ch;
                    }
                }

                PushLine(current);
            }
        }

        if(lines.empty())
            return false;

        return true;
    };

    if(autoFontSize)
    {
        float minSize = 0.1f;
        float maxSize = std::min(w, h);   // o algún límite como 256
        float bestSize = minSize;

        while(maxSize - minSize > 0.5f)
        {
            totalHeight = 0;
            float testSize = (minSize + maxSize) * 0.5f;
            currentFontSize = testSize;

#if defined(PLATFORM_PC)
            TTF_SetFontSize(font->font, testSize * windowScale);
            lineHeight = TTF_FontHeight(font->font);
#else
            lineHeight = testSize;
#endif

            // Vaciar líneas
            lines.clear();
            linesSize.clear();

            if(!CalculateTextSize())
            {
                return textResult;
            }

            float totalWidth = 0.0f;

            for(size_t i=0;i<linesSize.size();i++)
            {
                totalWidth = std::max(totalWidth, linesSize[i].x);

                totalHeight += linesSize[i].y;

                if(i+1<linesSize.size())
                    totalHeight += lineSpacing;
            }

            if(totalWidth <= w && totalHeight <= h)
            {
                bestSize = testSize;
                minSize = testSize;
            }
            else
            {
                maxSize = testSize;
            }
        }

        fontSize = bestSize;
        currentFontSize = bestSize;

    #if defined(PLATFORM_PC)
        TTF_SetFontSize(font->font, fontSize * windowScale);
        lineHeight = TTF_FontHeight(font->font);
    #endif

        if(!CalculateTextSize())
        {
            totalHeight = 0.0f;
            lines.clear();
            linesSize.clear();
            return textResult;
        }
    }
    else
    {
        if(!CalculateTextSize())
            return textResult;

        totalHeight = 0.0f;

        for(size_t i = 0; i < linesSize.size(); i++)
        {
            totalHeight += linesSize[i].y;

            if(i + 1 < linesSize.size())
                totalHeight += lineSpacing;
        }

        while(!lines.empty() && totalHeight > h)
        {
            totalHeight -= linesSize.back().y;

            if(lines.size() > 1)
                totalHeight -= lineSpacing;

            lines.pop_back();
            linesSize.pop_back();
        }
    }

    textResult.height = totalHeight;
    textResult.width =  w;

#if defined(PLATFORM_PC)

    std::vector<SDL_Surface*> surfs;

    for(size_t i = 0; i < lines.size(); i++)
    {
        surfs.push_back(
            TTF_RenderText_Solid(
                font->font,
                lines[i].c_str(),
                c
            )
        );
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

    drawY = fieldY + (h - totalHeight) * textAlignY;
    for(size_t i = 0; i < surfs.size(); i++)
    {
        SDL_Surface* conv = SDL_ConvertSurfaceFormat(
            surfs[i],
            SDL_PIXELFORMAT_RGBA32,
            0
        );

        SDL_FreeSurface(surfs[i]);

        GLuint texture = 0;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            conv->w,
            conv->h,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            conv->pixels
        );

        //-----------------------------------------------------
        // Alineación horizontal
        //-----------------------------------------------------

        float lineWidth = linesSize[i].x;

        // Si hay letterSpacing, el ancho real cambia
        if(letterSpacing > 0.0f)
        {
            lineWidth = -letterSpacing;

            for(char ch : lines[i])
            {
                int minx,maxx,miny,maxy,advance;

                if(TTF_GlyphMetrics(font->font, ch,
                    &minx,&maxx,&miny,&maxy,&advance) == 0)
                {
                    lineWidth += advance + letterSpacing;
                }
            }
        }

        float drawX = fieldX + (w - lineWidth) * textAlignX;

        //-----------------------------------------------------
        // Dibujar normalmente
        //-----------------------------------------------------

        if(letterSpacing <= 0.0f)
        {
            float x1 = drawX;
            float y1 = drawY;

            float x2 = x1 + conv->w;
            float y2 = y1 + conv->h;

            glEnable(GL_TEXTURE_2D);

            glColor4ub(255,255,255,255);

            glBegin(GL_QUADS);

                glTexCoord2f(0,0); glVertex3f(x1,y1, depth);
                glTexCoord2f(1,0); glVertex3f(x2,y1, depth);
                glTexCoord2f(1,1); glVertex3f(x2,y2, depth);
                glTexCoord2f(0,1); glVertex3f(x1,y2, depth);

            glEnd();

            glDisable(GL_TEXTURE_2D);
        }
        else
        {
            // Calcular el ancho total de la línea teniendo en cuenta el letterSpacing
            float totalWidth = -letterSpacing;

            for(char ch : lines[i])
            {
                int minx,maxx,miny,maxy,advance;

                if(TTF_GlyphMetrics(font->font, ch, &minx, &maxx, &miny, &maxy, &advance) == 0)
                    totalWidth += advance + letterSpacing;
            }

            float penX = drawX;
            float penY = drawY;

            for(char ch : lines[i])
            {
                char txt[2] = { ch, 0 };

                SDL_Surface* glyph = TTF_RenderText_Blended(
                    font->font,
                    txt,
                    c
                );

                if(!glyph)
                    continue;

                SDL_Surface* glyphConv = SDL_ConvertSurfaceFormat(
                    glyph,
                    SDL_PIXELFORMAT_RGBA32,
                    0
                );

                SDL_FreeSurface(glyph);

                GLuint glyphTex;

                glGenTextures(1, &glyphTex);
                glBindTexture(GL_TEXTURE_2D, glyphTex);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RGBA,
                    glyphConv->w,
                    glyphConv->h,
                    0,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    glyphConv->pixels
                );

                float x1 = penX;
                float y1 = penY;

                float x2 = x1 + glyphConv->w;
                float y2 = y1 + glyphConv->h;

                glEnable(GL_TEXTURE_2D);

                glColor4ub(255,255,255,255);

                glBegin(GL_QUADS);

                    glTexCoord2f(0,0); glVertex3f(x1,y1, depth);
                    glTexCoord2f(1,0); glVertex3f(x2,y1, depth);
                    glTexCoord2f(1,1); glVertex3f(x2,y2, depth);
                    glTexCoord2f(0,1); glVertex3f(x1,y2, depth);

                glEnd();

                glDisable(GL_TEXTURE_2D);

                int minx,maxx,miny,maxy,advance;

                if(TTF_GlyphMetrics(font->font, ch, &minx, &maxx, &miny, &maxy, &advance) == 0)
                    penX += advance + letterSpacing;
                else
                    penX += glyphConv->w + letterSpacing;

                glDeleteTextures(1, &glyphTex);

                SDL_FreeSurface(glyphConv);
            }
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        glDeleteTextures(1, &texture);

        SDL_FreeSurface(conv);

        drawY += linesSize[i].y + lineSpacing;
    }

    glPopMatrix();

    surfs.clear();

#elif defined(PLATFORM_3DS)

    float scale = fontSize / INITIAL_FONT_SIZE;   // tamaño base con el que cargaste la fuente
    if(scale <= 0.0f)
        scale = 1.0f;

    u32 clr = C2D_Color32(color.r, color.g, color.b, color.a);

    drawY = fieldY + (h - totalHeight) * textAlignY;
    for(size_t i = 0; i < lines.size(); i++)
    {
        C2D_Text txt;

        C2D_TextFontParse(
            &txt,
            font->font,
            textBuffer,
            lines[i].c_str()
        );
        C2D_TextOptimize(&txt);

        float tw, th;
        C2D_TextGetDimensions(
            &txt,
            scale,
            scale,
            &tw,
            &th
        );

        float lineWidth = tw;
        
        if(letterSpacing > 0.0f)
        {
            // Aproximación: añadimos el spacing entre caracteres
            lineWidth += (lines[i].size() > 1)
                ? (lines[i].size() - 1) * letterSpacing
                : 0.0f;
        }

        float drawX = fieldX + (w - lineWidth) * textAlignX;

        if(letterSpacing <= 0.0f)
        {
            C2D_DrawText(
                &txt,
                C2D_WithColor,
                drawX,
                drawY,
                depth,
                scale,
                scale,
                clr
            );
        }
        else
        {
            // Dibujado carácter a carácter
            float penX = drawX;

            for(char ch : lines[i])
            {
                char s[2] = { ch, 0 };

                C2D_Text glyph;

                C2D_TextFontParse(
                    &glyph,
                    font->font,
                    textBuffer,
                    s
                );
                C2D_TextOptimize(&glyph);

                float gw, gh;

                C2D_TextGetDimensions(
                    &glyph,
                    scale,
                    scale,
                    &gw,
                    &gh
                );

                C2D_DrawText(
                    &glyph,
                    C2D_WithColor,
                    penX,
                    drawY,
                    depth,
                    scale,
                    scale,
                    clr
                );

                penX += gw + letterSpacing;
            }
        }

        drawY += th + lineSpacing;
    }

#endif

    textResult.drawed = true;
    currentCharactersCount += textString.length();

    return textResult;
}

void D2D_InitTexts()
{
    if(!initialized || textsInitialized)
        return;
    currentCharactersCount = 0;
#if defined(PLATFORM_3DS)
    if(globalBuffer)
    {
        C2D_TextBufDelete(globalBuffer);
        globalBuffer = nullptr;
    }
    globalBuffer = C2D_TextBufNew(MAX_CHARACTERS_FT);
#endif
    textsInitialized = true;
    InitAllFonts();
    DrawConsole::InitConsole();
}

void D2D_TextsBegin()
{
    if(!initialized || !textsInitialized)
        return;
    currentCharactersCount = 0;
#if defined(PLATFORM_3DS)
    if(globalBuffer)
        C2D_TextBufClear(globalBuffer);
#endif
}

void D2D_TextsEnd()
{

}

void D2D_TextsDeleteAllBuffers()
{
    if(!textsInitialized)
        return;
#if defined(PLATFORM_3DS)
    if(globalBuffer)
        C2D_TextBufDelete(globalBuffer);
    globalBuffer = nullptr;
#endif
    currentCharactersCount = 0;
    textsInitialized = false;
    DrawConsole::EndConsole();
    CloseAllFonts();
}