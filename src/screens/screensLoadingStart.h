#pragma once

#if defined(PLATFORM_PC)
#include <utils/save_system/types.h>

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct SaveForC
{
    const char *name;
    uint8_t type;
    void *data;
} SaveForC;

typedef struct SaveForCArray
{
    SaveForC *list;
    unsigned int size;
} SaveForCArray;

SaveForCArray *loadInitial();

void setScreenValue(const char* name, void* value, SaveType type);

void freeLoadedList(SaveForCArray *list);

#ifdef __cplusplus
}
#endif

#endif