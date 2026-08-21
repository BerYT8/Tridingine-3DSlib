#pragma once

#include "../ints_defs.h"

typedef enum MemoryType
{
    MEMORY_KB,
    MEMORY_MB,
    MEMORY_GB,
    MEMORY_BYTES,
} MemoryType;

typedef struct MemoryInfo
{
    u64 applicationTotal;
    u64 applicationUsed;
    u64 applicationFree;
} MemoryInfo;

MemoryInfo memoryGetInfo(MemoryType type);

u64 memoryGetTotalUsed(MemoryType type);
u64 memoryGetAvailable(MemoryType type);
