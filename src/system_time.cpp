#include <sys/system_time.h>
#include <sys/system_language.h>
#include <string>
#include <vector>

#include <localization.h>
#include <maths.h>


#if defined(PLATFORM_PC)
#if defined(_WIN32)
#include <windows.h>

void GetDate(u8 &day, u8 &month, u16 &year, u8 &hour, u8 &min, u8 &sec)
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    day   = st.wDay;
    month = st.wMonth;
    year  = st.wYear;

    hour   = st.wHour;
    min = st.wMinute;
    sec = st.wSecond;
}
#elif defined(__linux__) || defined(__APPLE__)
#include <ctime>

void GetDate(u8 &day, u8 &month, u16 &year, u8 &hour, u8 &min, u8 &sec)
{
    time_t now = time(nullptr);
    tm lt;

    localtime_r(&now, &lt); // thread-safe (mejor que localtime)

    day   = lt.tm_mday;
    month = lt.tm_mon + 1;
    year  = lt.tm_year + 1900;

    hour   = lt.tm_hour;
    min = lt.tm_min;
    sec = lt.tm_sec;
}
#endif
#elif defined(PLATFORM_3DS)
#include <3ds.h>

void GetDate(u8 &day, u8 &month, u16 &year, u8 &hour, u8 &min, u8 &sec)
{
    time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t *)&unixTime);
    u8 h = timeStruct->tm_hour;
    u8 mn = timeStruct->tm_min;
    u8 s = timeStruct->tm_sec;
    u8 d = timeStruct->tm_mday;
    u8 m = timeStruct->tm_mon + 1;
    u16 y = timeStruct->tm_year +1900;

    day   = d;
    month = m;
    year  = y;

    hour = h;
    min = mn;
    sec = s;
}
#else
void GetDate(u8 &day, u8 &month, u16 &year, u8 &hour, u8 &min, u8 &sec)
{
    return;
}
#endif

static LOCALIZATION *loc;
static bool initialized = false;

static std::vector<std::string> months = std::vector<std::string>(12);
static std::vector<std::string> days = std::vector<std::string>(7);

bool Time_Init()
{
    if(initialized)
        return false;

    loc = Loc_Init();
    if(!loc)
        return false;

    Loc_AddLocale(loc, "engine/system_time/spanish.lang");
    Loc_AddLocale(loc, "engine/system_time/english.lang");
    Loc_AddLocale(loc, "engine/system_time/french.lang");
    Loc_AddLocale(loc, "engine/system_time/german.lang");
    Loc_AddLocale(loc, "engine/system_time/italian.lang");
    Loc_AddLocale(loc, "engine/system_time/portuguese.lang");
    Loc_AddLocale(loc, "engine/system_time/russian.lang");
    Loc_AddLocale(loc, "engine/system_time/japanese.lang");
    Loc_AddLocale(loc, "engine/system_time/korean.lang");
    Loc_AddLocale(loc, "engine/system_time/chinese_simplified.lang");
    Loc_AddLocale(loc, "engine/system_time/chinese_traditional.lang");

    std::string lang = System_GetCurrentLang();
    if(lang == "unknown")
    {
        System_SetCurrentLang("en-US");
        lang = "en-US";
    }
    Loc_LoadLocale(loc, lang.c_str());

    months = {
        std::string(Loc_GetText(loc, "JAN")),
        std::string(Loc_GetText(loc, "FEB")),
        std::string(Loc_GetText(loc, "MAR")),
        std::string(Loc_GetText(loc, "APR")),
        std::string(Loc_GetText(loc, "MAY")),
        std::string(Loc_GetText(loc, "JUN")),
        std::string(Loc_GetText(loc, "JUL")),
        std::string(Loc_GetText(loc, "AUG")),
        std::string(Loc_GetText(loc, "SEP")),
        std::string(Loc_GetText(loc, "OCT")),
        std::string(Loc_GetText(loc, "NOV")),
        std::string(Loc_GetText(loc, "DEC")),
    };

    days = {
        std::string(Loc_GetText(loc, "MON")),
        std::string(Loc_GetText(loc, "TUE")),
        std::string(Loc_GetText(loc, "WED")),
        std::string(Loc_GetText(loc, "THU")),
        std::string(Loc_GetText(loc, "FRI")),
        std::string(Loc_GetText(loc, "SAT")),
        std::string(Loc_GetText(loc, "SUN")),
    };

    initialized = true;
    
    return true;
}

bool isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

u8 GetDaysInMonth(u8 month, u16 year)
{
    switch (month)
    {
        case 1:  return 31;
        case 2:  return isLeapYear(year) ? 29 : 28;
        case 3:  return 31;
        case 4:  return 30;
        case 5:  return 31;
        case 6:  return 30;
        case 7:  return 31;
        case 8:  return 31;
        case 9:  return 30;
        case 10: return 31;
        case 11: return 30;
        case 12: return 31;
        default: return 0; // mes inválido
    }
}

Day GetDayFrom(u16 y, u8 m, u8 d)
{
    if (m < 3) {
        m += 12;
        y -= 1;
    }

    int K = y % 100;
    int J = y / 100;

    int h = (d + 13*(m + 1)/5 + K + K/4 + J/4 + 5*J) % 7;
    int day = ((h + 5) % 7) + 1;

    return (Day)day;
}

Day Time_GetDayFrom(u16 year, Month month, u8 day)
{
    u8 nmonth = clampf(month, 1, 12);
    return GetDayFrom(year, nmonth, clampf(day, 1, GetDaysInMonth(nmonth, year)));
}

const char* Time_GetMonthName(Month month)
{
    u8 nmonth = clampf(month, 1, 12)-1;
    if(months[nmonth].empty())
        return "";
    return months[nmonth].c_str();
}

const char* Time_GetDayName(Day day)
{
    int nday = clampf(day, 1, 7)-1;
    if(days[nday].empty())
        return "";
    return days[nday].c_str();
}

Time Time_GetCurrentTime()
{
    Time t = Time();
    u8 m = 0;
    GetDate(t.day, m, t.year, t.hour, t.minute, t.second);
    t.month = (Month)m;
    return t;
}

void Time_Exit()
{
    if(!initialized)
        return;
    months.clear();
    days.clear();
    Loc_Shutdown(loc);
}