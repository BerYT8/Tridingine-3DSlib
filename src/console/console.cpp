#include "drawConsole.h"

std::string VPrintf(const char *format, va_list args)
{
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    return std::string(buffer);
}

void printOnConsoleV(ScreenConsole console, LogType type, const char *format, va_list args)
{
    std::string text = VPrintf(format, args);
    DrawConsole::Print(console, type, text.c_str());
}

void printOnConsole(ScreenConsole console, LogType type, const char *format, ...)
{
    if (console != TOP_CONSOLE && console != BOTTOM_CONSOLE)
        return;
    va_list args;
    va_start(args, format);
    printOnConsoleV(console, type, format, args);
    va_end(args);
}

void clearConsole(ScreenConsole console)
{
    DrawConsole::ClearConsole(console);
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