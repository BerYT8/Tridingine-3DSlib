#include "drawConsole.h"
#include <sstream>

#define TEXT_FONT_SIZE 0.6f

int DrawConsole::c = 0;
float DrawConsole::sx = 0.0f;
float DrawConsole::sy = 0.0f;
std::vector<ConsoleText> DrawConsole::textsTop = {};
std::vector<ConsoleText> DrawConsole::textsBottom = {};
#if defined(PLATFORM_3DS)
C2D_Text *DrawConsole::infoText = nullptr;
#endif

void DrawConsole::SetConsole(int console)
{
    if (console != GSP_SCREEN_TOP && console != GSP_SCREEN_BOTTOM)
        return;
    c = console;
    sx = GSP_SCREEN_WIDTH;
    sy = console == GSP_SCREEN_TOP ? GSP_SCREEN_HEIGHT_TOP : GSP_SCREEN_HEIGHT_BOTTOM;
}

void DrawConsole::InitConsole()
{
#if defined(PLATFORM_3DS)
    infoText = new C2D_Text();
    C2D_TextBuf infoTextBuf = C2D_TextBufNew(256);
    C2D_TextParse(infoText, infoTextBuf, "Tridingine Console");
#endif
}

std::string wrapText(const std::string &input, float maxWidth)
{
    std::string result;
    std::string line;
    std::istringstream words(input);
    std::string word;

    while (words >> word)
    {
        std::string testLine = line;
        if (!line.empty())
            testLine += " ";
        testLine += word;

#if defined(PLATFORM_3DS)
        C2D_Text testText;
        C2D_TextBuf tmpBuf = C2D_TextBufNew(256);
        C2D_TextParse(&testText, tmpBuf, testLine.c_str());
        C2D_TextOptimize(&testText);

        if (testText.width > maxWidth)
        {
            result += line + "\n";
            line = word;
        }
        else
        {
            line = testLine;
        }

        C2D_TextBufDelete(tmpBuf);
#endif
    }

    if (!line.empty())
        result += line;

    return result;
}

void DrawConsole::Print(int console, LogType log, std::string text)
{
    if (console != GSP_SCREEN_TOP && console != GSP_SCREEN_BOTTOM)
        return;
#if defined(PLATFORM_3DS)
    C2D_Text *texti = new C2D_Text();
    C2D_TextBuf textBuf = C2D_TextBufNew(256);
    ConsoleText t;
    t.text = texti;
    t.type = log;

    float maxWidth = sx - 20; // margen de 10px a cada lado
    std::string wrapped = wrapText(text, maxWidth);
    C2D_TextParse(texti, textBuf, wrapped.c_str());
    C2D_TextOptimize(texti);

    float width, height;
    C2D_TextGetDimensions(texti, TEXT_FONT_SIZE, TEXT_FONT_SIZE, &width, &height);

    t.width = width;
    t.height = height;

    auto &texts = console == GSP_SCREEN_TOP ? textsTop : textsBottom;

    texts.push_back(t);

    if (texts.size() >= 10)
    {
        C2D_TextBufClear(texts[0].text->buf);
        C2D_TextBufDelete(texts[0].text->buf);
        delete texts[0].text;
        texts.erase(texts.begin());
    }
#elif defined(PLATFORM_PC)
    ConsoleText t;
    t.text = text;
    t.type = log;

    texts.push_back(t);

    if (texts.size() >= 10)
        texts.erase(texts.begin());
#endif
}

void DrawConsole::DrawTheConsole(int console)
{
    if (console != GSP_SCREEN_TOP && console != GSP_SCREEN_BOTTOM)
        return;
    auto &texts = console == GSP_SCREEN_TOP ? textsTop : textsBottom;
#if defined(PLATFORM_3DS)
    C2D_DrawText(infoText, C2D_WithColor, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, C2D_Color32(255, 0, 0, 255));
#elif defined(PLATFORM_PC)
    // Dibujar texto SDL
#endif
    float initialOffset = 20.0f;
    float topOffset = initialOffset;
#if defined(PLATFORM_3DS)
    auto size = texts.size();
    u32 white = C2D_Color32(255, 255, 255, 255);
    u32 red = C2D_Color32(255, 0, 0, 255);
    u32 yellow = C2D_Color32(0, 255, 255, 255);
    for (size_t i = 0; i < size; i++)
    {
        u32 &color = white;
        switch (texts[i].type)
        {
        case WARNING:
            color = yellow;
            break;
        case ERROR:
            color = red;
            break;
        default:
            color = white;
            break;
        }
        C2D_DrawText(texts[i].text, C2D_WithColor, 10.0f, 0.0f + topOffset, 0.5f, TEXT_FONT_SIZE, TEXT_FONT_SIZE, color);
        topOffset += texts[i].height + initialOffset;
    }
#elif defined(PLATFORM_PC)
    for (size_t i = 0; i < texts.size(); i++)
    {
        printf("%s\n", texts[i].text.c_str());
    }
#endif
}

void DrawConsole::ClearConsole(int console)
{
    if (console != GSP_SCREEN_TOP && console != GSP_SCREEN_BOTTOM)
        return;
    auto &texts = console == GSP_SCREEN_TOP ? textsTop : textsBottom;
#if defined(PLATFORM_3DS)
    for (size_t i = 0; i < texts.size(); i++)
    {
        C2D_TextBufClear(texts[i].text->buf);
        C2D_TextBufDelete(texts[i].text->buf);
        delete texts[i].text;
    }
#elif defined(PLATFORM_PC)
    // SDL
#endif
    texts.clear();
}

void DrawConsole::EndConsole()
{
#if defined(PLATFORM_3DS)
    C2D_TextBufClear(infoText->buf);
    C2D_TextBufDelete(infoText->buf);
    delete infoText;
#endif
    ClearConsole(GSP_SCREEN_TOP);
    ClearConsole(GSP_SCREEN_BOTTOM);
}