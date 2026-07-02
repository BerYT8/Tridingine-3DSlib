#pragma once

#include <stdio.h>
#include <stdbool.h>

typedef struct
{
    FILE *file;
} SaveFile;

#ifdef __cplusplus
extern "C"
{
#endif

    SaveFile save_open(const char *path);
    void save_close(SaveFile *save);

    void saveBool(SaveFile *save, const char *name, bool value);
    void saveInt(SaveFile *save, const char *name, int value);
    void saveFloat(SaveFile *save, const char *name, float value);
    void saveChar(SaveFile *save, const char *name, char value);
    void saveString(SaveFile *save, const char *name, const char *value);

#ifdef __cplusplus
}
#endif