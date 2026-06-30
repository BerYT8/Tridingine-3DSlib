#pragma once

#include <cstdarg>

#ifndef GSP_SCREEN_TOP
#define GSP_SCREEN_TOP 0
#endif

#ifndef GSP_SCREEN_BOTTOM
#define GSP_SCREEN_BOTTOM 1
#endif

enum LogType
{
    LOG,
    WARNING,
    ERROR
};

void setCurrentConsole(int console);

void printLog(const char *format, ...);
void printWarn(const char *format, ...);
void printError(const char *format, ...);

void printOnConsole(int console, LogType type, const char *format, ...);

void drawConsole(int console);

void consoleInit();

void consoleEnd();

void clearConsole(int console);