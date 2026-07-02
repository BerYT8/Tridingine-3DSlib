#include "delta_time.h"
#include <screens.h>

#ifdef PLATFORM_PC

#if defined(_WIN32)
    #include <windows.h>
#endif

    static LARGE_INTEGER frequency;
    static LARGE_INTEGER last_counter;
    static LARGE_INTEGER current_counter;

#elif defined(PLATFORM_3DS)

    #include <3ds.h>

    static u64 last_tick;
    static u64 current_tick;

#endif

static double delta_time = 0.0;
static double total_time = 0.0;

void dt_init(void)
{
#ifdef PLATFORM_PC

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&last_counter);

#elif defined(PLATFORM_3DS)

    last_tick = svcGetSystemTick();

#endif
}

void dt_update(void)
{
#ifdef PLATFORM_PC

    QueryPerformanceCounter(&current_counter);

    delta_time =
        (double)(current_counter.QuadPart - last_counter.QuadPart) /
        (double)frequency.QuadPart;

    last_counter = current_counter;

#elif defined(PLATFORM_3DS)

    current_tick = svcGetSystemTick();

    delta_time =
        (double)(current_tick - last_tick) / 268123480.0;

    last_tick = current_tick;

#endif

    if(!S2S_IsGamePaused())
        total_time += delta_time;
}

double dt_get(void)
{
    if(S2S_IsGamePaused())
        return 0;
    return delta_time;
}

double dt_total_time(void)
{
    return total_time;
}