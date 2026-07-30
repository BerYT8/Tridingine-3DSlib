#include "delta_time.h"
#include <screens.h>

#ifdef PLATFORM_PC

#if defined(_WIN32)
    #include <windows.h>
    static LARGE_INTEGER frequency;
    static LARGE_INTEGER last_counter;
    static LARGE_INTEGER current_counter;
#elif defined(__linux__) || defined(__APPLE__)
    #include <time.h>
    static struct timespec last_time;
    static struct timespec current_time;
#endif

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
    #if defined(_WIN32)
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&last_counter);
    #elif defined(__linux__) || defined(__APPLE__)
        clock_gettime(CLOCK_MONOTONIC, &last_time);
    #endif
#elif defined(PLATFORM_3DS)
    last_tick = svcGetSystemTick();
#endif
}

void dt_update(void)
{
#ifdef PLATFORM_PC
    #if defined(_WIN32)
        QueryPerformanceCounter(&current_counter);

        delta_time =
            (double)(current_counter.QuadPart - last_counter.QuadPart) /
            (double)frequency.QuadPart;

        last_counter = current_counter;
    #elif defined(__linux__) || defined(__APPLE__)
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        
        double seconds = (double)(current_time.tv_sec - last_time.tv_sec);
        double nanoseconds = (double)(current_time.tv_nsec - last_time.tv_nsec);
        
        delta_time = seconds + nanoseconds * 1e-9;
        last_time = current_time;
    #endif
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
