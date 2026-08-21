#include <sys/system_memory.h>

#include <stddef.h>
#include <stdint.h>
#include "screens/screensValues.h"

#if defined(PLATFORM_3DS)

#include <3ds.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h> // Necesario para sbrk
#include <reent.h> // Cabecera para funciones reentrantes de newlib

#elif defined(PLATFORM_PC)

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>

#elif defined(__linux__)

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#elif defined(__APPLE__)

#include <mach/mach.h>
#include <sys/sysctl.h>

#endif

#endif


/* ============================================================
 * MEMORY API
 *
 * Semántica:
 *
 *     applicationTotal
 *     applicationUsed
 *     applicationFree
 *
 * SIEMPRE representan el mismo espacio de memoria.
 *
 * 3DS:
 *     Heap de malloc/newlib.
 *
 * PC:
 *     memoria física del sistema.
 *
 * ============================================================ */


/* ------------------------------------------------------------
 * Safe subtraction
 * ------------------------------------------------------------ */

static u64 memorySubtract(
    u64 a,
    u64 b)
{
    if (b >= a)
        return 0;

    return a - b;
}


/* ------------------------------------------------------------
 * Convert bytes to requested unit
 * ------------------------------------------------------------ */

static u64 memoryConvert(
    u64 bytes,
    MemoryType type)
{
    switch (type)
    {
        case MEMORY_KB:
            return bytes / 1024ULL;

        case MEMORY_MB:
            return bytes / (1024ULL * 1024ULL);

        case MEMORY_GB:
            return bytes / (1024ULL * 1024ULL * 1024ULL);

        case MEMORY_BYTES:
        default:
            return bytes;
    }
}


/* ============================================================
 * 3DS
 * ============================================================ */

#if defined(PLATFORM_3DS)

static MemoryInfo memoryGetInfo3DS(void)
{
    MemoryInfo info = {0};

    if (g_heap_start == NULL)
    {
        /*
         * No es lo ideal, pero evita devolver basura.
         * Si llegamos aquí, las asignaciones anteriores a esta llamada
         * no podrán contabilizarse.
         */
        g_heap_start = (u8*)_sbrk_r(_REENT, 0);
    }

    u8* heap_current = (u8*)_sbrk_r(_REENT, 0);

    u32 total_heap = envGetHeapSize();

    u32 used_heap = 0;

    if (heap_current >= g_heap_start)
        used_heap = (u32)(heap_current - g_heap_start);

    u32 free_heap = 0;

    if (used_heap < total_heap)
        free_heap = total_heap - used_heap;

    info.applicationTotal = (u64)total_heap;
    info.applicationUsed  = (u64)used_heap;
    info.applicationFree  = (u64)free_heap;

    return info;
}

#endif



/* ============================================================
 * WINDOWS
 * ============================================================ */

#if defined(PLATFORM_PC) && defined(_WIN32)

static MemoryInfo memoryGetInfoWindows(void)
{
    MemoryInfo info = {0};

    MEMORYSTATUSEX status;

    ZeroMemory(
        &status,
        sizeof(status)
    );

    status.dwLength =
        sizeof(status);

    if (!GlobalMemoryStatusEx(&status))
        return info;

    u64 total =
        (u64)status.ullTotalPhys;

    u64 free =
        (u64)status.ullAvailPhys;

    if (free > total)
        free = total;

    info.applicationTotal = total;
    info.applicationFree  = free;
    info.applicationUsed  =
        memorySubtract(total, free);

    return info;
}

#endif


/* ============================================================
 * LINUX
 * ============================================================ */

#if defined(PLATFORM_PC) && defined(__linux__)

static u64 memoryReadLinuxMeminfo(
    const char *name)
{
    FILE *file =
        fopen(
            "/proc/meminfo",
            "r"
        );

    if (file == NULL)
        return 0;

    char line[256];

    u64 value = 0;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        char key[64];
        unsigned long long number = 0;
        char unit[32];

        int result =
            sscanf(
                line,
                "%63[^:]: %llu %31s",
                key,
                &number,
                unit
            );

        if (result >= 2 &&
            strcmp(key, name) == 0)
        {
            value =
                (u64)number * 1024ULL;

            break;
        }
    }

    fclose(file);

    return value;
}


static MemoryInfo memoryGetInfoLinux(void)
{
    MemoryInfo info = {0};

    u64 total =
        memoryReadLinuxMeminfo(
            "MemTotal"
        );

    u64 available =
        memoryReadLinuxMeminfo(
            "MemAvailable"
        );

    if (total == 0)
        return info;

    if (available > total)
        available = total;

    info.applicationTotal = total;
    info.applicationFree  = available;
    info.applicationUsed  =
        memorySubtract(total, available);

    return info;
}

#endif


/* ============================================================
 * macOS
 * ============================================================ */

#if defined(PLATFORM_PC) && defined(__APPLE__)

static MemoryInfo memoryGetInfoMacOS(void)
{
    MemoryInfo info = {0};

    uint64_t totalMemory = 0;

    size_t totalSize =
        sizeof(totalMemory);

    int totalMib[2] =
    {
        CTL_HW,
        HW_MEMSIZE
    };

    if (sysctl(
            totalMib,
            2,
            &totalMemory,
            &totalSize,
            NULL,
            0) != 0)
    {
        return info;
    }

    u64 total =
        (u64)totalMemory;


    mach_msg_type_number_t count =
        HOST_VM_INFO64_COUNT;

    vm_statistics64_data_t vmStats;

    kern_return_t result =
        host_statistics64(
            mach_host_self(),
            HOST_VM_INFO64,
            (host_info64_t)&vmStats,
            &count
        );

    if (result != KERN_SUCCESS)
        return info;


    uint64_t pageSize = 0;

    size_t pageSizeSize =
        sizeof(pageSize);

    int pageMib[2] =
    {
        CTL_HW,
        HW_PAGESIZE
    };

    if (sysctl(
            pageMib,
            2,
            &pageSize,
            &pageSizeSize,
            NULL,
            0) != 0)
    {
        return info;
    }


    u64 availablePages =
        (u64)vmStats.free_count +
        (u64)vmStats.inactive_count;

    u64 available =
        availablePages * (u64)pageSize;

    if (available > total)
        available = total;

    info.applicationTotal = total;
    info.applicationFree  = available;
    info.applicationUsed  =
        memorySubtract(total, available);

    return info;
}

#endif


/* ============================================================
 * PUBLIC API
 * ============================================================ */

MemoryInfo memoryGetInfo(
    MemoryType type)
{
    MemoryInfo info = {0};

    if(screensInitialized)
    {

#if defined(PLATFORM_3DS)

        info =
            memoryGetInfo3DS();


#elif defined(PLATFORM_PC)

#if defined(_WIN32)

        info =
            memoryGetInfoWindows();

#elif defined(__linux__)

        info =
            memoryGetInfoLinux();

#elif defined(__APPLE__)

        info =
            memoryGetInfoMacOS();

#endif

#endif


        /*
        * --------------------------------------------------------
        * Validación
        * --------------------------------------------------------
        */

        if (info.applicationUsed >
            info.applicationTotal)
        {
            info.applicationUsed =
                info.applicationTotal;
        }

        info.applicationFree =
            memorySubtract(
                info.applicationTotal,
                info.applicationUsed
            );


        /*
        * --------------------------------------------------------
        * Conversión
        * --------------------------------------------------------
        */

        info.applicationTotal =
            memoryConvert(
                info.applicationTotal,
                type
            );

        info.applicationUsed =
            memoryConvert(
                info.applicationUsed,
                type
            );

        info.applicationFree =
            memoryConvert(
                info.applicationFree,
                type
            );
    }
    else
    {
        info.applicationFree = 0;
        info.applicationTotal = 0;
        info.applicationUsed = 0;
    }

    return info;
}


/* ============================================================
 * TOTAL USED
 * ============================================================ */

u64 memoryGetTotalUsed(
    MemoryType type)
{
    if(screensInitialized)
    {
        MemoryInfo info =
            memoryGetInfo(type);

        return info.applicationUsed;
    }
    return 0;
}


/* ============================================================
 * AVAILABLE
 * ============================================================ */

u64 memoryGetAvailable(
    MemoryType type)
{
    if(screensInitialized)
    {
        MemoryInfo info =
            memoryGetInfo(type);

        return info.applicationFree;
    }
    return 0;
}
