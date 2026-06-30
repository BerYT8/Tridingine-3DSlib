#include "drawConsole.h"

void setCurrentConsole(int console)
{
    DrawConsole::SetConsole(console);
}

std::string VPrintf(const char *format, va_list args)
{
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    return std::string(buffer);
}

void printOnConsoleV(int console, LogType type, const char *format, va_list args)
{
    std::string text = VPrintf(format, args);
    DrawConsole::Print(console, type, text.c_str());
}

void printOnConsole(int console, LogType type, const char *format, ...)
{
    if (console != GSP_SCREEN_TOP && console != GSP_SCREEN_BOTTOM)
        return;
    va_list args;
    va_start(args, format);
    printOnConsoleV(console, type, format, args);
    va_end(args);
}

void clearConsole(int console)
{
    DrawConsole::ClearConsole(console);
}

void consoleInit()
{
    DrawConsole::InitConsole();
}

void drawConsole(int console)
{
    DrawConsole::DrawTheConsole(console);
}

void consoleEnd()
{
    DrawConsole::EndConsole();
}

void printLog(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printOnConsoleV(DrawConsole::GetCurrentConsole(), LOG, format, args);
    va_end(args);
}

void printWarn(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printOnConsoleV(DrawConsole::GetCurrentConsole(), WARNING, format, args);
    va_end(args);
}

void printError(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printOnConsoleV(DrawConsole::GetCurrentConsole(), ERROR, format, args);
    va_end(args);
}