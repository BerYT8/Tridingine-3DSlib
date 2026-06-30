#pragma once

#include <stdbool.h>
#include <ints_defs.h>

typedef enum Month
{
    JAN = 1,
    FEB,
    MAR,
    APR,
    MAY,
    JUN,
    JUL,
    AUG,
    SEP,
    OCT,
    NOV,
    DEC,
} Month;

typedef enum Day
{
    MON = 1,
    TUE,
    WED,
    THU,
    FRI,
    SAT,
    SUN,
} Day;

typedef struct Time
{
    u16 year;
    Month month;
    u8 day;
    
    u8 hour;
    u8 minute;
    u8 second;
} Time;

#ifdef __cplusplus
extern "C"
{
#endif

bool Time_Init();

Time Time_GetCurrentTime();

Day Time_GetDayFrom(u16 year, Month month, u8 day);

const char* Time_GetMonthName(Month month);
const char* Time_GetDayName(Day day);

void Time_Exit();

#ifdef __cplusplus
}
#endif