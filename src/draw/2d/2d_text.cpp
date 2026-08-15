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
#include <algorithm>
#include <cmath>
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


// ============================================================================
// Utilidades de tamaño de fuente
// ============================================================================

/*
 * fontSize representa SIEMPRE la altura deseada de la línea.
 *
 * Ejemplo:
 *
 *     D2D_DrawText(..., 24.0f, ...)
 *
 * significa:
 *
 *     "quiero que la altura real de la línea sea aproximadamente 24 px".
 *
 * El backend puede necesitar un tamaño de fuente diferente para conseguirlo.
 *
 * Por ejemplo:
 *
 *     fontSize solicitado = 24
 *     tamaño real TTF      = 31
 *     altura TTF           = 24
 *
 * En ese caso se utilizará 31 internamente.
 */


#if defined(PLATFORM_PC)

static float GetPCFontHeight(
    D2D_Font* font,
    float realFontSize
)
{
    if(!font || !font->font)
        return 0.0f;

    if(realFontSize <= 0.0f)
        return 0.0f;

    int pixelFontSize = std::max(
        1,
        (int)std::round(realFontSize * windowScale)
    );

    TTF_SetFontSize(
        font->font,
        pixelFontSize
    );

    // Convertimos SOLO aquí de píxeles escalados
    // a unidades lógicas.
    return (float)TTF_FontHeight(font->font) / windowScale;
}


static float FindPCFontSizeForHeight(
    D2D_Font* font,
    float wantedHeight
)
{
    if(!font || !font->font || wantedHeight <= 0.0f)
        return 1.0f;

    float low = 1.0f;

    float high = std::max(
        wantedHeight * 2.0f,
        (float)INITIAL_FONT_SIZE
    );

    float highHeight =
        GetPCFontHeight(
            font,
            high
        );

    while(
        highHeight < wantedHeight &&
        high < 4096.0f
    )
    {
        high *= 2.0f;

        highHeight =
            GetPCFontHeight(
                font,
                high
            );
    }

    for(int i = 0; i < 20; i++)
    {
        float mid =
            (low + high) * 0.5f;

        float height =
            GetPCFontHeight(
                font,
                mid
            );

        if(height < wantedHeight)
            low = mid;
        else
            high = mid;
    }

    float bestSize = high;

    float bestDifference =
        std::fabs(
            GetPCFontHeight(
                font,
                bestSize
            ) - wantedHeight
        );

    const float candidates[] =
    {
        low,
        high,
        (low + high) * 0.5f,
        std::floor(low),
        std::ceil(low),
        std::floor(high),
        std::ceil(high)
    };

    for(float candidate : candidates)
    {
        if(candidate <= 0.0f)
            continue;

        float height =
            GetPCFontHeight(
                font,
                candidate
            );

        float difference =
            std::fabs(
                height - wantedHeight
            );

        if(difference < bestDifference)
        {
            bestDifference = difference;
            bestSize = candidate;
        }
    }

    return bestSize;
}

#elif defined(PLATFORM_3DS)

static float Get3DSFontHeight(
    D2D_Font* font,
    C2D_TextBuf textBuffer,
    const std::string& text,
    float realFontSize
)
{
    if(!font || !font->font || !textBuffer)
        return 0.0f;

    if(realFontSize <= 0.0f)
        return 0.0f;

    C2D_Text txt;

    C2D_TextFontParse(
        &txt,
        font->font,
        textBuffer,
        text.c_str()
    );

    C2D_TextOptimize(&txt);

    float scale =
        realFontSize / (float)INITIAL_FONT_SIZE;

    if(scale <= 0.0f)
        scale = 0.001f;

    float tw = 0.0f;
    float th = 0.0f;

    C2D_TextGetDimensions(
        &txt,
        scale,
        scale,
        &tw,
        &th
    );

    return th;
}


static float Find3DSFontSizeForHeight(
    D2D_Font* font,
    C2D_TextBuf textBuffer,
    const std::string& referenceText,
    float wantedHeight
)
{
    if(
        !font ||
        !font->font ||
        !textBuffer ||
        wantedHeight <= 0.0f
    )
    {
        return 1.0f;
    }

    /*
     * En C2D la escala es proporcional al tamaño base de la fuente,
     * pero NO asumimos que INITIAL_FONT_SIZE == altura real.
     *
     * Medimos realmente mediante C2D_TextGetDimensions().
     */

    float low = 0.1f;

    float high = std::max(
        wantedHeight * 2.0f,
        (float)INITIAL_FONT_SIZE
    );

    float highHeight = Get3DSFontHeight(
        font,
        textBuffer,
        referenceText,
        high
    );

    while(highHeight < wantedHeight && high < 4096.0f)
    {
        high *= 2.0f;

        highHeight = Get3DSFontHeight(
            font,
            textBuffer,
            referenceText,
            high
        );
    }

    /*
     * Búsqueda binaria.
     */

    for(int i = 0; i < 16; i++)
    {
        float mid = (low + high) * 0.5f;

        float height = Get3DSFontHeight(
            font,
            textBuffer,
            referenceText,
            mid
        );

        if(height < wantedHeight)
            low = mid;
        else
            high = mid;
    }

    /*
     * Refinamiento final.
     */

    float bestSize = high;

    float bestDifference =
        std::fabs(
            Get3DSFontHeight(
                font,
                textBuffer,
                referenceText,
                bestSize
            ) - wantedHeight
        );

    const float candidates[] =
    {
        low,
        high,
        (low + high) * 0.5f,
        std::floor(low),
        std::ceil(low),
        std::floor(high),
        std::ceil(high)
    };

    for(float candidate : candidates)
    {
        if(candidate <= 0.0f)
            continue;

        float height = Get3DSFontHeight(
            font,
            textBuffer,
            referenceText,
            candidate
        );

        float difference =
            std::fabs(height - wantedHeight);

        if(difference < bestDifference)
        {
            bestDifference = difference;
            bestSize = candidate;
        }
    }

    return bestSize;
}

#endif


// ============================================================================
// Abrir fuente
// ============================================================================

D2D_Font *D2D_OpenFont_Buf(const char* path, bool del)
{
    D2D_Font *ft = new D2D_Font();

    if(!ft)
        return nullptr;

#if defined(PLATFORM_PC)

    PAK_FILE* f =
        PAKL_LoadFile(
            (std::string(path) + ".ttf").c_str()
        );

    if(!f)
    {
        delete ft;
        return nullptr;
    }

    PAKL_fseek(f, 0, SEEK_END);

    long tamano = PAKL_ftell(f);

    PAKL_rewind(f);

    if(tamano <= 0)
    {
        PAKL_CloseFile(f);
        delete ft;
        return nullptr;
    }

    void* buffer = malloc(tamano);

    if(!buffer)
    {
        PAKL_CloseFile(f);
        delete ft;
        return nullptr;
    }

    size_t leidos =
        PAKL_fread(
            buffer,
            1,
            tamano,
            f
        );

    PAKL_CloseFile(f);

    SDL_RWops* rw = nullptr;

    if(leidos == (size_t)tamano)
    {
        rw =
            SDL_RWFromMem(
                (void*)buffer,
                tamano
            );
    }

    if(!rw)
    {
        SDL_Log(
            "Error al crear RWops: %s",
            SDL_GetError()
        );

        free(buffer);
        delete ft;

        return nullptr;
    }

    ft->font =
        TTF_OpenFontRW(
            rw,
            1,
            INITIAL_FONT_SIZE
        );

    if(!ft->font)
    {
        delete ft;
        free(buffer);

        return nullptr;
    }

    ft->buffer = buffer;

#elif defined(PLATFORM_3DS)

    std::string full =
        getRomfsPath(
            (std::string(path) + ".bcfnt").c_str()
        );

    printf(
        "%s\n",
        full.c_str()
    );

    ft->font =
        C2D_FontLoad(
            full.c_str()
        );

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


void D2D_CloseFont_Buf(
    D2D_Font *font,
    bool del
)
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


// ============================================================================
// DrawText
// ============================================================================

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

    auto res =
        D2D_DrawText_Buf(
            text,
            font,
            fontSize,
            color,
            x,
            y,
            depth,
            w,
            h,
            alignX,
            alignY,
            textAlignX,
            textAlignY,
            letterSpacing,
            lineSpacing,
            wrap,
            false,
            TOP_CONSOLE
        );

    return res.drawed
        ? D2D_OK
        : D2D_ERROR;
}


// ============================================================================
// Buffers
// ============================================================================

void InitConsoleBuffs()
{
#if defined(PLATFORM_3DS)

    consoleTopBuffer =
        C2D_TextBufNew(
            MAX_CHARACTERS_FT
        );

    consoleBotBuffer =
        C2D_TextBufNew(
            MAX_CHARACTERS_FT
        );

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
    if(
        console != TOP_CONSOLE &&
        console != BOTTOM_CONSOLE
    )
        return;

#if defined(PLATFORM_3DS)

    auto buf =
        console == TOP_CONSOLE
            ? consoleTopBuffer
            : consoleBotBuffer;

    if(buf)
        C2D_TextBufClear(buf);

#endif
}


// ============================================================================
// D2D_DrawText_Buf
// ============================================================================

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

    if(
        depth < -1 ||
        depth > 1 ||
        !font
    )
        return textResult;

    if(!font->font)
        return textResult;


    // ========================================================================
    // Preparar texto
    // ========================================================================

    std::string textString = text;

    if(
        textString.length() >=
        MAX_CHARACTERS_FT - currentCharactersCount
    )
    {
        textString.erase(
            MAX_CHARACTERS_FT - 3
        );

        textString += "...";
    }


#if defined(PLATFORM_PC)

    /*
     * A partir de aquí TODO el layout se hace en píxeles reales
     * de la ventana.
     */

    const float scaleFactor = windowScale;

    x *= scaleFactor;
    y *= scaleFactor;
    w *= scaleFactor;
    h *= scaleFactor;

    x +=
        currScreen == TOP
            ? topInitialPointX
            : bottomInitialPointX;

    y +=
        currScreen == TOP
            ? topInitialPointY
            : bottomInitialPointY;

#endif


    alignX =
        clampf(
            alignX,
            0.f,
            1.f
        );

    alignY =
        clampf(
            alignY,
            0.f,
            1.f
        );


    bool autoFontSize =
        fontSize < 0.0f;


    /*
     * Si fontSize es positivo:
     *
     *     fontSize = ALTURA DESEADA
     *
     * Si fontSize es negativo:
     *
     *     se mantiene el auto-fit original.
     */

    if(!autoFontSize && fontSize < 0)
        fontSize = 0;


    if(w < 0)
        w = 0;

    if(h < 0)
        h = 0;


    // ========================================================================
    // Campo
    // ========================================================================

    float fieldX =
        x - w * alignX;

    float fieldY =
        y - h * alignY;


    std::vector<std::string> lines;
    std::vector<Vec2> linesSize;


    float lineHeight = 0.0f;


#if defined(PLATFORM_3DS)

    C2D_TextBuf textBuffer =
        console
            ? (
                consoleN == TOP_CONSOLE
                    ? consoleTopBuffer
                    : consoleBotBuffer
              )
            : globalBuffer;

    if(!textBuffer)
        return textResult;

#endif


    /*
     * Este es el tamaño REAL de fuente que terminará usando el backend.
     *
     * fontSize = altura deseada.
     *
     * currentFontSize = tamaño interno real.
     */

    float currentFontSize =
        fontSize;


    // ========================================================================
    // Medición
    // ========================================================================

    auto MeasureText =
        [&](const std::string& str,
            float& tw,
            float& th)
    {
    #if defined(PLATFORM_PC)

        int mw = 0;
        int mh = 0;

        TTF_SizeText(
            font->font,
            str.c_str(),
            &mw,
            &mh
        );

        /*
        * NO dividir por windowScale aquí.
        *
        * w, x, y, h ya están escalados.
        * Las SDL_Surface también estarán escaladas.
        *
        * Por tanto, todo debe permanecer en píxeles
        * escalados durante el cálculo del layout.
        */

        tw = (float)mw;
        th = (float)mh;

    #elif defined(PLATFORM_3DS)

        C2D_Text txt;

        C2D_TextParse(
            &txt,
            textBuffer,
            str.c_str()
        );

        C2D_TextOptimize(&txt);

        float scale =
            currentFontSize /
            (float)INITIAL_FONT_SIZE;

        if(scale <= 0.0f)
            scale = 0.001f;

        C2D_TextGetDimensions(
            &txt,
            scale,
            scale,
            &tw,
            &th
        );

    #endif
    };


    // ========================================================================
    // Buscar el tamaño real para obtener la altura solicitada
    // ========================================================================

    auto ResolveFontSize =
        [&](float wantedHeight,
            const std::string& referenceText)
            -> float
    {
        if(wantedHeight <= 0.0f)
            return 1.0f;

#if defined(PLATFORM_PC)

        return FindPCFontSizeForHeight(
            font,
            wantedHeight
        );

#elif defined(PLATFORM_3DS)

        return Find3DSFontSizeForHeight(
            font,
            textBuffer,
            referenceText,
            wantedHeight
        );

#endif
    };


    // ========================================================================
    // Crear líneas
    // ========================================================================

    auto PushLine =
        [&](const std::string& str)
    {
        float tw = 0.0f;
        float th = 0.0f;

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


    float totalHeight = 0.0f;
    float drawY = 0.0f;


    // ========================================================================
    // Calcular líneas
    // ========================================================================

    auto CalculateTextSize =
        [&]() -> bool
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

            while(
                std::getline(
                    ss,
                    line,
                    '\n'
                )
            )
            {
                PushLine(line);
            }
        }
        else if(wrap == WORD_WRAP_MODE)
        {
            std::stringstream ss(textString);

            std::string paragraph;

            while(
                std::getline(
                    ss,
                    paragraph,
                    '\n'
                )
            )
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

                    if(
                        tw <= w ||
                        current.empty()
                    )
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

            while(
                std::getline(
                    ss,
                    paragraph,
                    '\n'
                )
            )
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

                    if(
                        tw <= w ||
                        current.empty()
                    )
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


        for(size_t i = 0; i < linesSize.size(); i++)
        {
            totalHeight +=
                linesSize[i].y;

            if(i + 1 < linesSize.size())
                totalHeight +=
                    lineSpacing;
        }


        return true;
    };


    // ========================================================================
    // AUTO FONT SIZE
    // ========================================================================

    if(autoFontSize)
    {
        /*
         * Auto-fit original:
         *
         * buscamos el MAYOR tamaño de fuente que entre en w/h.
         *
         * Pero ahora cada tamaño candidato también se interpreta
         * como altura deseada y se convierte al tamaño real del backend.
         */

        float minSize = 0.1f;

        float maxSize =
            std::min(w, h);

        if(maxSize <= 0.0f)
            return textResult;

        float bestSize =
            minSize;


        while(
            maxSize - minSize > 0.5f
        )
        {
            float testSize =
                (minSize + maxSize) * 0.5f;

            /*
             * Elegimos una referencia provisional.
             *
             * En la mayoría de fuentes cualquier línea no vacía sirve
             * para determinar la altura.
             */

            std::string referenceText =
                textString.empty()
                    ? "A"
                    : textString;


            currentFontSize =
                ResolveFontSize(
                    testSize,
                    referenceText
                );


#if defined(PLATFORM_PC)

            TTF_SetFontSize(
                font->font,
                std::max(
                    1,
                    (int)std::round(
                        currentFontSize *
                        windowScale
                    )
                )
            );

#endif


            if(!CalculateTextSize())
                return textResult;


            float totalWidth = 0.0f;

            totalHeight = 0.0f;


            for(size_t i = 0;
                i < linesSize.size();
                i++)
            {
                totalWidth =
                    std::max(
                        totalWidth,
                        linesSize[i].x
                    );

                totalHeight +=
                    linesSize[i].y;

                if(i + 1 < linesSize.size())
                    totalHeight +=
                        lineSpacing;
            }


            if(
                totalWidth <= w &&
                totalHeight <= h
            )
            {
                bestSize = testSize;

                minSize = testSize;
            }
            else
            {
                maxSize = testSize;
            }
        }


        /*
         * `bestSize` es la ALTURA deseada.
         *
         * Convertimos finalmente esa altura al tamaño real.
         */

        fontSize =
            bestSize;

        currentFontSize =
            ResolveFontSize(
                fontSize,
                textString.empty()
                    ? "A"
                    : textString
            );


#if defined(PLATFORM_PC)

        TTF_SetFontSize(
            font->font,
            std::max(
                1,
                (int)std::round(
                    currentFontSize *
                    windowScale
                )
            )
        );

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
        // ================================================================
        // FONT SIZE NORMAL
        // ================================================================

        /*
         * Aquí está el cambio principal:
         *
         * fontSize NO se utiliza directamente.
         *
         * Primero buscamos qué tamaño real necesita la fuente para
         * conseguir una altura igual o aproximadamente igual a fontSize.
         */

        std::string referenceText =
            textString.empty()
                ? "A"
                : textString;


        currentFontSize =
            ResolveFontSize(
                fontSize,
                referenceText
            );


#if defined(PLATFORM_PC)

        TTF_SetFontSize(
            font->font,
            std::max(
                1,
                (int)std::round(
                    currentFontSize *
                    windowScale
                )
            )
        );

#endif


        if(!CalculateTextSize())
            return textResult;


        /*
         * Recalcular la altura usando las dimensiones REALES.
         */

        totalHeight = 0.0f;

        for(size_t i = 0;
            i < linesSize.size();
            i++)
        {
            totalHeight +=
                linesSize[i].y;

            if(i + 1 < linesSize.size())
                totalHeight +=
                    lineSpacing;
        }


        /*
         * Mantener el comportamiento original:
         * si no cabe verticalmente, eliminamos líneas del final.
         */

        while(
            !lines.empty() &&
            totalHeight > h
        )
        {
            totalHeight -=
                linesSize.back().y;

            if(lines.size() > 1)
                totalHeight -=
                    lineSpacing;

            lines.pop_back();

            linesSize.pop_back();
        }
    }


    // ========================================================================
    // Resultado
    // ========================================================================

    textResult.height =
        totalHeight;

    textResult.width =
        w;


    // ========================================================================
    // PC
    // ========================================================================

#if defined(PLATFORM_PC)

    SDL_Color c;

    c.r = color.r;
    c.g = color.g;
    c.b = color.b;
    c.a = color.a;


    /*
     * MUY IMPORTANTE:
     *
     * currentFontSize es el tamaño REAL de TTF.
     *
     * fontSize sigue siendo la altura solicitada.
     */

    TTF_SetFontSize(
        font->font,
        std::max(
            1,
            (int)std::round(
                currentFontSize *
                windowScale
            )
        )
    );


    std::vector<SDL_Surface*> surfs;


    for(size_t i = 0;
        i < lines.size();
        i++)
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
        1
    );


    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();

    glPushMatrix();

    glUseProgram(0);


    drawY =
        fieldY +
        (h - totalHeight) *
        textAlignY;


    for(size_t i = 0;
        i < surfs.size();
        i++)
    {
        SDL_Surface* conv =
            SDL_ConvertSurfaceFormat(
                surfs[i],
                SDL_PIXELFORMAT_RGBA32,
                0
            );


        SDL_FreeSurface(
            surfs[i]
        );


        GLuint texture = 0;

        glGenTextures(
            1,
            &texture
        );

        glBindTexture(
            GL_TEXTURE_2D,
            texture
        );


        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_LINEAR
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            GL_LINEAR
        );


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


        // ================================================================
        // Alineación horizontal
        // ================================================================

        float lineWidth =
            linesSize[i].x;


        if(letterSpacing > 0.0f)
        {
            lineWidth =
                -letterSpacing;


            for(char ch : lines[i])
            {
                int minx;
                int maxx;
                int miny;
                int maxy;
                int advance;


                if(
                    TTF_GlyphMetrics(
                        font->font,
                        ch,
                        &minx,
                        &maxx,
                        &miny,
                        &maxy,
                        &advance
                    ) == 0
                )
                {
                    lineWidth +=
                        advance +
                        letterSpacing;
                }
            }
        }


        float drawX =
            fieldX +
            (w - lineWidth) *
            textAlignX;


        // ================================================================
        // Sin letter spacing
        // ================================================================

        if(letterSpacing <= 0.0f)
        {
            float x1 = drawX;
            float y1 = drawY;

            float x2 =
                x1 + conv->w;

            float y2 =
                y1 + conv->h;


            glEnable(
                GL_TEXTURE_2D
            );

            glColor4ub(
                255,
                255,
                255,
                255
            );


            glBegin(
                GL_QUADS
            );

                glTexCoord2f(
                    0,
                    0
                );

                glVertex3f(
                    x1,
                    y1,
                    depth
                );


                glTexCoord2f(
                    1,
                    0
                );

                glVertex3f(
                    x2,
                    y1,
                    depth
                );


                glTexCoord2f(
                    1,
                    1
                );

                glVertex3f(
                    x2,
                    y2,
                    depth
                );


                glTexCoord2f(
                    0,
                    1
                );

                glVertex3f(
                    x1,
                    y2,
                    depth
                );

            glEnd();


            glDisable(
                GL_TEXTURE_2D
            );
        }
        else
        {
            // ============================================================
            // Letter spacing
            // ============================================================

            float totalWidth =
                -letterSpacing;


            for(char ch : lines[i])
            {
                int minx;
                int maxx;
                int miny;
                int maxy;
                int advance;


                if(
                    TTF_GlyphMetrics(
                        font->font,
                        ch,
                        &minx,
                        &maxx,
                        &miny,
                        &maxy,
                        &advance
                    ) == 0
                )
                {
                    totalWidth +=
                        advance +
                        letterSpacing;
                }
            }


            float penX = drawX;
            float penY = drawY;


            for(char ch : lines[i])
            {
                char txt[2] =
                {
                    ch,
                    0
                };


                SDL_Surface* glyph =
                    TTF_RenderText_Blended(
                        font->font,
                        txt,
                        c
                    );


                if(!glyph)
                    continue;


                SDL_Surface* glyphConv =
                    SDL_ConvertSurfaceFormat(
                        glyph,
                        SDL_PIXELFORMAT_RGBA32,
                        0
                    );


                SDL_FreeSurface(
                    glyph
                );


                GLuint glyphTex;


                glGenTextures(
                    1,
                    &glyphTex
                );


                glBindTexture(
                    GL_TEXTURE_2D,
                    glyphTex
                );


                glTexParameteri(
                    GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR
                );

                glTexParameteri(
                    GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR
                );


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

                float x2 =
                    x1 + glyphConv->w;

                float y2 =
                    y1 + glyphConv->h;


                glEnable(
                    GL_TEXTURE_2D
                );


                glColor4ub(
                    255,
                    255,
                    255,
                    255
                );


                glBegin(
                    GL_QUADS
                );

                    glTexCoord2f(
                        0,
                        0
                    );

                    glVertex3f(
                        x1,
                        y1,
                        depth
                    );


                    glTexCoord2f(
                        1,
                        0
                    );

                    glVertex3f(
                        x2,
                        y1,
                        depth
                    );


                    glTexCoord2f(
                        1,
                        1
                    );

                    glVertex3f(
                        x2,
                        y2,
                        depth
                    );


                    glTexCoord2f(
                        0,
                        1
                    );

                    glVertex3f(
                        x1,
                        y2,
                        depth
                    );

                glEnd();


                glDisable(
                    GL_TEXTURE_2D
                );


                int minx;
                int maxx;
                int miny;
                int maxy;
                int advance;


                if(
                    TTF_GlyphMetrics(
                        font->font,
                        ch,
                        &minx,
                        &maxx,
                        &miny,
                        &maxy,
                        &advance
                    ) == 0
                )
                {
                    penX +=
                        advance +
                        letterSpacing;
                }
                else
                {
                    penX +=
                        glyphConv->w +
                        letterSpacing;
                }


                glDeleteTextures(
                    1,
                    &glyphTex
                );


                SDL_FreeSurface(
                    glyphConv
                );
            }
        }


        glBindTexture(
            GL_TEXTURE_2D,
            0
        );


        glDeleteTextures(
            1,
            &texture
        );


        SDL_FreeSurface(
            conv
        );


        drawY +=
            linesSize[i].y +
            lineSpacing;
    }


    glPopMatrix();

    surfs.clear();


// ============================================================================
// 3DS
// ============================================================================

#elif defined(PLATFORM_3DS)

    float scale =
        currentFontSize /
        (float)INITIAL_FONT_SIZE;

    if(scale <= 0.0f)
        scale = 0.001f;


    u32 clr =
        C2D_Color32(
            color.r,
            color.g,
            color.b,
            color.a
        );


    drawY =
        fieldY +
        (h - totalHeight) *
        textAlignY;


    for(size_t i = 0;
        i < lines.size();
        i++)
    {
        C2D_Text txt;


        C2D_TextFontParse(
            &txt,
            font->font,
            textBuffer,
            lines[i].c_str()
        );


        C2D_TextOptimize(
            &txt
        );


        float tw;
        float th;


        C2D_TextGetDimensions(
            &txt,
            scale,
            scale,
            &tw,
            &th
        );


        float lineWidth =
            tw;


        if(letterSpacing > 0.0f)
        {
            lineWidth +=
                lines[i].size() > 1
                    ? (
                        lines[i].size() - 1
                    ) * letterSpacing
                    : 0.0f;
        }


        float drawX =
            fieldX +
            (w - lineWidth) *
            textAlignX;


        // ================================================================
        // Sin letter spacing
        // ================================================================

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
            // ============================================================
            // Letter spacing
            // ============================================================

            float penX =
                drawX;


            for(char ch : lines[i])
            {
                char s[2] =
                {
                    ch,
                    0
                };


                C2D_Text glyph;


                C2D_TextFontParse(
                    &glyph,
                    font->font,
                    textBuffer,
                    s
                );


                C2D_TextOptimize(
                    &glyph
                );


                float gw;
                float gh;


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


                penX +=
                    gw +
                    letterSpacing;
            }
        }


        drawY +=
            th +
            lineSpacing;
    }

#endif


    textResult.drawed = true;

    currentCharactersCount +=
        textString.length();


    return textResult;
}


// ============================================================================
// Inicialización
// ============================================================================

void D2D_InitTexts()
{
    if(!initialized || textsInitialized)
        return;


    currentCharactersCount = 0;


#if defined(PLATFORM_3DS)

    if(globalBuffer)
    {
        C2D_TextBufDelete(
            globalBuffer
        );

        globalBuffer = nullptr;
    }


    globalBuffer =
        C2D_TextBufNew(
            MAX_CHARACTERS_FT
        );

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
        C2D_TextBufClear(
            globalBuffer
        );

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
        C2D_TextBufDelete(
            globalBuffer
        );

    globalBuffer = nullptr;

#endif


    currentCharactersCount = 0;

    textsInitialized = false;

    DrawConsole::EndConsole();

    CloseAllFonts();
}