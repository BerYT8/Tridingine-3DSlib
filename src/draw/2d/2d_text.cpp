#include <draw/2d/2d_shapes.h>

#include "2d_vals.h"

#include <maths.h>

#ifndef INITIAL_FONT_SIZE
#define INITIAL_FONT_SIZE 24
#endif

/*
 * Compensación horizontal usada EXCLUSIVAMENTE para decidir el WRAP
 * en PC.
 *
 * No modifica:
 *  - el tamaño visual
 *  - las posiciones
 *  - el alineamiento
 *  - las texturas
 *
 * 3DS hace wrap ligeramente antes que PC, por lo que PC necesita
 * considerar su anchura ligeramente mayor durante el cálculo.
 */
#ifndef PC_WRAP_WIDTH_SCALE
#define PC_WRAP_WIDTH_SCALE 1.27f
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
// Utilidades
// ============================================================================

#if defined(PLATFORM_PC)

/*
 * IMPORTANTE:
 *
 * En PC el TTF SIEMPRE se mantiene cargado/renderizado a
 * INITIAL_FONT_SIZE.
 *
 * fontSize NO cambia el TTF.
 *
 * Ejemplo:
 *
 *     INITIAL_FONT_SIZE = 24
 *     fontSize          = 36
 *
 * El TTF continúa siendo de 24.
 *
 * Después se calcula:
 *
 *     scale = 36 / alturaRealDeLaFuenteBase
 *
 * y la textura se dibuja con ese scale.
 *
 * Esto hace que PC funcione igual conceptualmente que 3DS:
 *
 *     fuente base -> medir -> escalar resultado.
 *
 * No hacemos TTF_SetFontSize() durante el layout.
 */


static void EnsurePCBaseFont(
    D2D_Font* font
)
{
    if(!font || !font->font)
        return;

    /*
     * SDL_ttf necesita internamente un tamaño entero para rasterizar.
     *
     * Este es el ÚNICO sitio donde el tamaño de la fuente llega a SDL.
     *
     * El layout posterior siempre trabaja con floats.
     */

    if(TTF_FontHeight(font->font) <= 0)
    {
        TTF_SetFontSize(
            font->font,
            INITIAL_FONT_SIZE
        );
    }
}


static float GetPCBaseFontHeight(
    D2D_Font* font
)
{
    if(!font || !font->font)
        return 0.0f;

    EnsurePCBaseFont(font);

    return (float)TTF_FontHeight(
        font->font
    );
}


/*
 * Devuelve la escala necesaria para que la altura REAL de la fuente
 * sea igual a fontSize.
 *
 * Ejemplo:
 *
 *     altura TTF real = 28
 *     fontSize        = 24
 *
 *     scale = 24 / 28
 *
 * De esta manera la textura acaba teniendo 24 unidades de alto.
 */
static float GetPCTextScale(
    D2D_Font* font,
    float fontSize
)
{
    const float baseHeight =
        GetPCBaseFontHeight(font);

    if(baseHeight <= 0.0f)
        return 0.001f;

    if(fontSize <= 0.0f)
        return 0.001f;

    return fontSize / baseHeight;
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
        realFontSize /
        (float)INITIAL_FONT_SIZE;

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

    float low = 0.1f;

    float high =
        std::max(
            wantedHeight * 2.0f,
            (float)INITIAL_FONT_SIZE
        );

    float highHeight =
        Get3DSFontHeight(
            font,
            textBuffer,
            referenceText,
            high
        );

    while(
        highHeight < wantedHeight &&
        high < 4096.0f
    )
    {
        high *= 2.0f;

        highHeight =
            Get3DSFontHeight(
                font,
                textBuffer,
                referenceText,
                high
            );
    }

    for(int i = 0; i < 16; i++)
    {
        float mid =
            (low + high) * 0.5f;

        float height =
            Get3DSFontHeight(
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
        (low + high) * 0.5f
    };

    for(float candidate : candidates)
    {
        if(candidate <= 0.0f)
            continue;

        float height =
            Get3DSFontHeight(
                font,
                textBuffer,
                referenceText,
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

#endif


// ============================================================================
// Abrir fuente
// ============================================================================

D2D_Font* D2D_OpenFont_Buf(
    const char* path,
    bool del
)
{
    D2D_Font* ft =
        new D2D_Font();

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

    PAKL_fseek(
        f,
        0,
        SEEK_END
    );

    long tamano =
        PAKL_ftell(f);

    PAKL_rewind(f);

    if(tamano <= 0)
    {
        PAKL_CloseFile(f);
        delete ft;
        return nullptr;
    }

    void* buffer =
        malloc(tamano);

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
                buffer,
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

    /*
     * SIEMPRE abrimos el TTF a INITIAL_FONT_SIZE.
     *
     * No volvemos a cambiarlo en D2D_DrawText_Buf.
     */

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


D2D_Font* D2D_OpenFont(
    const char* path
)
{
    return D2D_OpenFont_Buf(
        path,
        true
    );
}


void D2D_CloseFont_Buf(
    D2D_Font* font,
    bool del
)
{
    if(!font)
        return;

    if(!font->deletable && del)
        return;


#if defined(PLATFORM_PC)

    TTF_CloseFont(
        font->font
    );

    free(
        font->buffer
    );


#elif defined(PLATFORM_3DS)

    C2D_FontFree(
        font->font
    );

#endif


    delete font;
}


void D2D_CloseFont(
    D2D_Font* font
)
{
    D2D_CloseFont_Buf(
        font,
        true
    );
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
    if(
        !initialized ||
        !textsInitialized
    )
    {
        return D2D_NOT_INITIALIZED;
    }

    if(!isValidScreen())
        return D2D_ERROR;

    if(
        currentCharactersCount >=
        MAX_CHARACTERS_FT - 3
    )
    {
        return D2D_ERROR;
    }

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
        C2D_TextBufDelete(
            consoleTopBuffer
        );

    if(consoleBotBuffer)
        C2D_TextBufDelete(
            consoleBotBuffer
        );

    consoleTopBuffer = nullptr;
    consoleBotBuffer = nullptr;

#endif
}


void ClearConsoleBuf(
    ScreenConsole console
)
{
    if(
        console != TOP_CONSOLE &&
        console != BOTTOM_CONSOLE
    )
    {
        return;
    }


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
    textResult.height = 0.0f;
    textResult.width = 0.0f;


    if(
        depth < -1.0f ||
        depth > 1.0f ||
        !font
    )
    {
        return textResult;
    }

    if(!font->font)
        return textResult;


    // ========================================================================
    // Preparar texto
    // ========================================================================

    std::string textString =
        text ? text : "";


    if(
        textString.length() >=
        MAX_CHARACTERS_FT -
        currentCharactersCount
    )
    {
        textString.erase(
            MAX_CHARACTERS_FT - 3
        );

        textString += "...";
    }


#if defined(PLATFORM_PC)

    /*
     * x/y/w/h pasan a píxeles físicos.
     *
     * PERO el tamaño lógico del texto NO.
     *
     * fontSize continúa siendo una unidad lógica.
     */

    const float scaleFactor =
        windowScale;

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

    fontSize *= scaleFactor;

#endif


    alignX =
        clampf(
            alignX,
            0.0f,
            1.0f
        );

    alignY =
        clampf(
            alignY,
            0.0f,
            1.0f
        );


    const bool autoFontSize =
        fontSize < 0.0f;


    if(
        !autoFontSize &&
        fontSize < 0.0f
    )
    {
        fontSize = 0.0f;
    }


    if(w < 0.0f)
        w = 0.0f;

    if(h < 0.0f)
        h = 0.0f;


    // ========================================================================
    // Campo
    // ========================================================================

    const float fieldX =
        x - w * alignX;

    const float fieldY =
        y - h * alignY;


    std::vector<std::string> lines;

    std::vector<Vec2> linesSize;


    float totalHeight = 0.0f;


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


    // ========================================================================
    // Tamaño del texto
    // ========================================================================

#if defined(PLATFORM_PC)

    /*
     * La fuente BASE permanece siempre a INITIAL_FONT_SIZE.
     */

    EnsurePCBaseFont(font);

    const float baseFontHeight =
        GetPCBaseFontHeight(font);

#endif


    float currentFontScale = 1.0f;


#if defined(PLATFORM_PC)

    if(fontSize > 0.0f)
    {
        currentFontScale =
            fontSize /
            baseFontHeight;
    }

    if(currentFontScale <= 0.0f)
        currentFontScale = 0.001f;


#elif defined(PLATFORM_3DS)

    float currentFontSize =
        fontSize;

#endif


    // ========================================================================
    // MeasureText
    // ========================================================================

    auto MeasureText =
        [&](const std::string& str,
            float& tw,
            float& th)
    {
#if defined(PLATFORM_PC)

        /*
         * TTF_SizeUTF8 devuelve la métrica de la cadena en la fuente base.
         *
         * SDL_ttf utiliza ints internamente para esta operación.
         *
         * En cuanto recibimos el resultado lo convertimos a float.
         *
         * Todo el layout posterior permanece en float.
         */

        int baseW = 0;
        int baseH = 0;

        TTF_SizeUTF8(
            font->font,
            str.c_str(),
            &baseW,
            &baseH
        );

        tw =
            (float)baseW *
            currentFontScale;

        th =
            (float)baseH *
            currentFontScale;


#elif defined(PLATFORM_3DS)

        C2D_Text txt;

        C2D_TextParse(
            &txt,
            textBuffer,
            str.c_str()
        );

        C2D_TextOptimize(
            &txt
        );

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


        /*
         * letterSpacing forma parte del tamaño REAL de la línea.
         */

        if(letterSpacing > 0.0f)
        {
            const float spacing =
#if defined(PLATFORM_PC)
                letterSpacing * currentFontScale;
#else
                letterSpacing;
#endif

            const size_t count =
                str.size();

            if(count > 1)
            {
                tw +=
                    (float)(count - 1) *
                    spacing;
            }
        }
    };


    // ========================================================================
    // PushLine
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


    // ========================================================================
    // WrapWidth
    // ========================================================================

    /*
     * Devuelve la anchura que se utilizará EXCLUSIVAMENTE para decidir
     * si una línea entra dentro del campo de wrap.
     *
     * La anchura visual real NO se modifica.
     *
     * De esta forma:
     *
     *     linesSize[i].x -> tamaño visual real
     *
     *     GetWrapWidth() -> tamaño usado para decidir wrap
     *
     * Esto evita alterar alineamiento o dibujo.
     */

    auto GetWrapWidth =
        [&](float visualWidth) -> float
    {
#if defined(PLATFORM_PC)

        return
            visualWidth *
            PC_WRAP_WIDTH_SCALE;

#else

        return visualWidth;

#endif
    };


    // ========================================================================
    // Calcular texto y WRAP
    // ========================================================================

    auto CalculateTextSize =
        [&]() -> bool
    {
#if defined(PLATFORM_3DS)

        if(textBuffer)
            C2D_TextBufClear(
                textBuffer
            );

#endif

        lines.clear();
        linesSize.clear();

        totalHeight = 0.0f;


        // ====================================================================
        // SIN WRAP
        // ====================================================================

        if(wrap == WRAP_NONE)
        {
            std::stringstream ss(
                textString
            );

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


        // ====================================================================
        // WORD WRAP
        // ====================================================================

        else if(wrap == WORD_WRAP_MODE)
        {
            std::stringstream ss(
                textString
            );

            std::string paragraph;

            while(
                std::getline(
                    ss,
                    paragraph,
                    '\n'
                )
            )
            {
                std::istringstream iss(
                    paragraph
                );

                std::string word;
                std::string current;


                while(iss >> word)
                {
                    const std::string test =
                        current.empty()
                            ? word
                            : current + " " + word;


                    float tw = 0.0f;
                    float th = 0.0f;


                    MeasureText(
                        test,
                        tw,
                        th
                    );


                    /*
                     * tw es el ancho visual REAL.
                     *
                     * Para decidir wrap usamos una métrica ligeramente
                     * compensada en PC para igualar el comportamiento de
                     * 3DS.
                     */

                    const float wrapWidth =
                        GetWrapWidth(tw);


                    if(
                        wrapWidth <= w ||
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


                /*
                 * Incluso si el párrafo está vacío debemos conservar
                 * la línea.
                 */

                PushLine(current);
            }
        }


        // ====================================================================
        // CHAR WRAP
        // ====================================================================

        else
        {
            std::stringstream ss(
                textString
            );

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
                    std::string test =
                        current;

                    test += ch;


                    float tw = 0.0f;
                    float th = 0.0f;


                    MeasureText(
                        test,
                        tw,
                        th
                    );


                    const float wrapWidth =
                        GetWrapWidth(tw);


                    if(
                        wrapWidth <= w ||
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


        totalHeight = 0.0f;


        for(size_t i = 0;
            i < linesSize.size();
            ++i)
        {
            totalHeight +=
                linesSize[i].y;

            if(i + 1 < linesSize.size())
            {
                totalHeight +=
                    lineSpacing;
            }
        }


        return true;
    };


    // ========================================================================
    // AUTO FONT SIZE
    // ========================================================================

    if(autoFontSize)
    {
        float minSize = 0.1f;

        float maxSize =
            std::min(
                w,
                h
            );


        if(maxSize <= 0.0f)
            return textResult;


        float bestSize =
            minSize;


        while(
            maxSize - minSize > 0.01f
        )
        {
            const float testSize =
                (minSize + maxSize) *
                0.5f;


#if defined(PLATFORM_PC)

            currentFontScale =
                testSize /
                baseFontHeight;

            if(currentFontScale <= 0.0f)
                currentFontScale = 0.001f;

#elif defined(PLATFORM_3DS)

            currentFontSize =
                testSize;

#endif


            if(!CalculateTextSize())
                return textResult;


            float totalWidth = 0.0f;

            float measuredHeight = 0.0f;


            for(size_t i = 0;
                i < linesSize.size();
                ++i)
            {
                /*
                 * Para el tamaño real del texto usamos la métrica visual,
                 * no la compensación de wrap.
                 */

                totalWidth =
                    std::max(
                        totalWidth,
                        linesSize[i].x
                    );

                measuredHeight +=
                    linesSize[i].y;

                if(i + 1 < linesSize.size())
                {
                    measuredHeight +=
                        lineSpacing;
                }
            }


            if(
                totalWidth <= w &&
                measuredHeight <= h
            )
            {
                bestSize =
                    testSize;

                minSize =
                    testSize;
            }
            else
            {
                maxSize =
                    testSize;
            }
        }


        fontSize =
            bestSize;


#if defined(PLATFORM_PC)

        currentFontScale =
            fontSize /
            baseFontHeight;

        if(currentFontScale <= 0.0f)
            currentFontScale = 0.001f;

#elif defined(PLATFORM_3DS)

        currentFontSize =
            fontSize;

#endif


        if(!CalculateTextSize())
            return textResult;
    }


    // ========================================================================
    // TAMAÑO NORMAL
    // ========================================================================

    else
    {
#if defined(PLATFORM_PC)

        /*
         * NO TTF_SetFontSize().
         *
         * La fuente continúa siendo INITIAL_FONT_SIZE.
         */

        currentFontScale =
            fontSize /
            baseFontHeight;

        if(currentFontScale <= 0.0f)
            currentFontScale = 0.001f;


#elif defined(PLATFORM_3DS)

        currentFontSize =
            fontSize;

#endif


        /*
         * El wrap se calcula usando la métrica visual y la compensación
         * horizontal exclusiva de wrap en PC.
         */

        if(!CalculateTextSize())
            return textResult;


        totalHeight = 0.0f;


        for(size_t i = 0;
            i < linesSize.size();
            ++i)
        {
            totalHeight +=
                linesSize[i].y;

            if(i + 1 < linesSize.size())
            {
                totalHeight +=
                    lineSpacing;
            }
        }


        /*
         * Si no cabe verticalmente eliminamos líneas.
         */

        while(
            !lines.empty() &&
            totalHeight > h
        )
        {
            totalHeight -=
                linesSize.back().y;

            if(lines.size() > 1)
            {
                totalHeight -=
                    lineSpacing;
            }

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
     * NO modificamos el tamaño del TTF.
     *
     * Las superficies se generan a INITIAL_FONT_SIZE.
     */

    EnsurePCBaseFont(font);


    std::vector<SDL_Surface*> surfs;

    surfs.reserve(
        lines.size()
    );


    for(size_t i = 0;
        i < lines.size();
        ++i)
    {
        SDL_Surface* surface =
            TTF_RenderText_Blended(
                font->font,
                lines[i].c_str(),
                c
            );

        surfs.push_back(
            surface
        );
    }


    glDepthMask(GL_TRUE);

    glDisable(
        GL_DEPTH_TEST
    );


    glMatrixMode(
        GL_PROJECTION
    );

    glLoadIdentity();


    glOrtho(
        0,
        wwidth,
        wheight,
        0,
        -1,
        1
    );


    glMatrixMode(
        GL_MODELVIEW
    );

    glLoadIdentity();

    glPushMatrix();

    glUseProgram(0);


    float drawY =
        fieldY +
        (h - totalHeight) *
        textAlignY;


    for(size_t i = 0;
        i < lines.size();
        ++i)
    {
        SDL_Surface* surface =
            surfs[i];


        if(!surface)
        {
            drawY +=
                linesSize[i].y +
                lineSpacing;

            continue;
        }


        SDL_Surface* conv =
            SDL_ConvertSurfaceFormat(
                surface,
                SDL_PIXELFORMAT_RGBA32,
                0
            );


        SDL_FreeSurface(
            surface
        );


        if(!conv)
        {
            drawY +=
                linesSize[i].y +
                lineSpacing;

            continue;
        }


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


        // ====================================================================
        // Ancho lógico de la línea
        // ====================================================================

        float lineWidth =
            linesSize[i].x;


        /*
         * IMPORTANTE:
         *
         * lineWidth es el ancho VISUAL REAL.
         *
         * NO contiene PC_WRAP_WIDTH_SCALE.
         *
         * Por tanto la compensación de wrap no afecta al alineamiento.
         */


        float drawX =
            fieldX +
            (w - lineWidth) *
            textAlignX;


        // ====================================================================
        // Sin letter spacing
        // ====================================================================

        if(letterSpacing <= 0.0f)
        {
            /*
             * La superficie es base.
             *
             * La escalamos físicamente mediante los vértices.
             */

            const float texW =
                (float)conv->w *
                currentFontScale;

            const float texH =
                (float)conv->h *
                currentFontScale;


            const float x1 =
                drawX;

            const float y1 =
                drawY;


            const float x2 =
                x1 + texW;

            const float y2 =
                y1 + texH;


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
                    0.0f,
                    0.0f
                );

                glVertex3f(
                    x1,
                    y1,
                    depth
                );


                glTexCoord2f(
                    1.0f,
                    0.0f
                );

                glVertex3f(
                    x2,
                    y1,
                    depth
                );


                glTexCoord2f(
                    1.0f,
                    1.0f
                );

                glVertex3f(
                    x2,
                    y2,
                    depth
                );


                glTexCoord2f(
                    0.0f,
                    1.0f
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


        // ====================================================================
        // Letter spacing
        // ====================================================================

        else
        {
            /*
             * En este caso generamos cada glifo.
             *
             * Cada glifo sigue siendo rasterizado a la fuente base.
             *
             * La posición y el tamaño final se escalan mediante floats.
             */

            float penX =
                drawX;


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


                if(!glyphConv)
                    continue;


                GLuint glyphTex = 0;


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


                const float glyphW =
                    (float)glyphConv->w *
                    currentFontScale;

                const float glyphH =
                    (float)glyphConv->h *
                    currentFontScale;


                const float x1 =
                    penX;

                const float y1 =
                    drawY;


                const float x2 =
                    x1 + glyphW;

                const float y2 =
                    y1 + glyphH;


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
                        0.0f,
                        0.0f
                    );

                    glVertex3f(
                        x1,
                        y1,
                        depth
                    );


                    glTexCoord2f(
                        1.0f,
                        0.0f
                    );

                    glVertex3f(
                        x2,
                        y1,
                        depth
                    );


                    glTexCoord2f(
                        1.0f,
                        1.0f
                    );

                    glVertex3f(
                        x2,
                        y2,
                        depth
                    );


                    glTexCoord2f(
                        0.0f,
                        1.0f
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
                    /*
                     * advance es un valor de métrica de SDL y por tanto
                     * es entero por limitación de SDL_ttf.
                     *
                     * Lo convertimos inmediatamente a float y desde
                     * aquí TODO el layout permanece en float.
                     */

                    penX +=
                        (float)advance *
                        currentFontScale;

                    penX +=
                        letterSpacing *
                        currentFontScale;
                }
                else
                {
                    penX +=
                        glyphW +
                        letterSpacing *
                        currentFontScale;
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


        /*
         * drawY utiliza la altura lógica ya escalada.
         */

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


    float drawY =
        fieldY +
        (h - totalHeight) *
        textAlignY;


    for(size_t i = 0;
        i < lines.size();
        ++i)
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


        float tw = 0.0f;
        float th = 0.0f;


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
                        (float)(lines[i].size() - 1) *
                        letterSpacing
                    )
                    : 0.0f;
        }


        float drawX =
            fieldX +
            (w - lineWidth) *
            textAlignX;


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


                float gw = 0.0f;
                float gh = 0.0f;


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
    if(
        !initialized ||
        textsInitialized
    )
    {
        return;
    }


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
    if(
        !initialized ||
        !textsInitialized
    )
    {
        return;
    }


    currentCharactersCount = 0;


#if defined(PLATFORM_3DS)

    if(globalBuffer)
    {
        C2D_TextBufClear(
            globalBuffer
        );
    }

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