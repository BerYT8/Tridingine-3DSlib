#include "drawConsole.h"
#include <sstream>
#include <draw/2d/2d_shapes.h>
#include "../draw/2d/2d_vals.h"
#include <screens.h>

#define TEXT_FONT_SIZE 0.6f

static bool cInitialized;

static D2D_Font *font;

ScreenConsole DrawConsole::c = ScreenConsole::TOP_CONSOLE;
float DrawConsole::sx = 0.0f;
float DrawConsole::sy = 0.0f;
std::vector<ConsoleText> DrawConsole::textsTop = {};
std::vector<ConsoleText> DrawConsole::textsBottom = {};

void DrawConsole::SetConsole(ScreenConsole console)
{
    if (console != TOP_CONSOLE && console != BOTTOM_CONSOLE)
        return;
    c = console;
}

void DrawConsole::InitConsole()
{
    if(cInitialized)
        return;
    font = D2D_OpenFont("engine/fonts/arial");
    if(font)
    {
        cInitialized = true;
        InitConsoleBuffs();
    }
    
}

void DrawConsole::Print(ScreenConsole console, LogType log, std::string text)
{
    if(!cInitialized)
        return;
    if (console != TOP_CONSOLE && console != BOTTOM_CONSOLE)
        return;

    Vec2 sSize = console == TOP_CONSOLE ? S2S_GetScreenSize(TOP) : S2S_GetScreenSize(BOTTOM);

    ConsoleText t;
    t.text = text;
    t.type = log;

    std::vector<ConsoleText> &texts = console == TOP_CONSOLE ? textsTop : textsBottom;

    texts.push_back(t);

    if (texts.size() >= 10)
        texts.erase(texts.begin());
}

void DrawConsole::DrawTheConsole(ScreenConsole console)
{
    if(!cInitialized)
        return;
    if (console != TOP_CONSOLE && console != BOTTOM_CONSOLE)
        return;

    Vec2 sSize = console == TOP_CONSOLE ? S2S_GetScreenSize(TOP) : S2S_GetScreenSize(BOTTOM);
    
    auto &texts = console == TOP_CONSOLE ? textsTop : textsBottom;

    D2D_DrawText_Buf("Console v1.0.0", font, 7, Color_Red, 0, 0, 1.0f, 100, 100, 0, 0, 0, 0, 0, 0, WRAP_NONE, true, console);

    float initialOffset = 20.0f;
    float topOffset = initialOffset;
    
    auto size = texts.size();
    Color white = Color_MakeColor(255, 255, 255, 255);
    Color red = Color_MakeColor(255, 0, 0, 255);
    Color yellow = Color_MakeColor(0, 255, 255, 255);
    for (size_t i = 0; i < size; i++)
    {
        Color &color = white;
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
        
        auto text = D2D_DrawText_Buf(texts[i].text.c_str(), font, 18, color, 0, initialOffset + topOffset, 1.0f, sSize.x, sSize.y, 0, 0, 0, 0, 0, 0, WORD_WRAP_MODE, true, console);

        if(text.drawed)
            topOffset += text.height + initialOffset;
    }
}

void DrawConsole::ClearConsole(ScreenConsole console)
{
    if (console != TOP_CONSOLE && console != BOTTOM_CONSOLE)
        return;
    auto &texts = console == TOP_CONSOLE ? textsTop : textsBottom;
    texts.clear();
    ClearConsoleBuf(console);
}

void DrawConsole::EndConsole()
{
    if(!cInitialized)
        return;
    ClearConsole(TOP_CONSOLE);
    ClearConsole(BOTTOM_CONSOLE);
    if(font)
        D2D_CloseFont(font);
    EndConsoleBuffs();
    cInitialized = false;
}