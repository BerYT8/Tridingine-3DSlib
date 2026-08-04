#pragma once

#include <console/console.h>

#include <vector>
#include <string>

#if defined(PLATFORM_3DS)
#include <3ds.h>
#include <citro2d.h>
#endif

    
struct ConsoleText
{
    std::string text = "";
    LogType type = LOG;
};

class DrawConsole
{
private:
    static ScreenConsole c;
    static float sx, sy;
    static std::vector<ConsoleText> textsTop;
    static std::vector<ConsoleText> textsBottom;
#if defined(PLATFORM_3DS)
    static C2D_Text *infoText;
#endif
public:
    DrawConsole() = default;
    ~DrawConsole() = default;

    static ScreenConsole GetCurrentConsole()
    {
        return c;
    }

    static void SetConsole(ScreenConsole console);

    static void InitConsole();

    static void Print(ScreenConsole console, LogType log, std::string text);
    static void DrawTheConsole(ScreenConsole console);
    static void ClearConsole(ScreenConsole console);

    static void EndConsole();
};