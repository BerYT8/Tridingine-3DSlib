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
#if defined(PLATFORM_3DS)
    C2D_Text *text = nullptr;
#elif defined(PLATFORM_PC)
    std::string text = "";
#endif
    LogType type = LOG;
    float width = 0;
    float height = 0;
};

class DrawConsole
{
private:
    static int c;
    static float sx, sy;
    static std::vector<ConsoleText> textsTop;
    static std::vector<ConsoleText> textsBottom;
#if defined(PLATFORM_3DS)
    static C2D_Text *infoText;
#endif
public:
    DrawConsole() = default;
    ~DrawConsole() = default;

    static int GetCurrentConsole()
    {
        return c;
    }

    static void SetConsole(int console);

    static void InitConsole();

    static void Print(int console, LogType log, std::string text);
    static void DrawTheConsole(int console);
    static void ClearConsole(int console);

    static void EndConsole();
};