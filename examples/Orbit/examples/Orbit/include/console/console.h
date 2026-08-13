#pragma once

#include <stdarg.h>

typedef enum LogType
{
    LOG,
    WARNING,
    ERROR
} LogType;

typedef enum ScreenConsole
{
    TOP_CONSOLE,
    BOTTOM_CONSOLE,
} ScreenConsole;

#ifdef __cplusplus
extern "C"
{
#endif

void printLog(const char *format, ...);
void printWarn(const char *format, ...);
void printError(const char *format, ...);

void printOnConsole(ScreenConsole console, LogType type, const char *format, ...);

void clearConsole(ScreenConsole console);

#ifdef __cplusplus
}
#endif