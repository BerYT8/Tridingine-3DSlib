#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    char *data;
    uint32_t size;

} SaveDataRaw;

#ifdef __cplusplus
extern "C"
{
#endif

    SaveDataRaw load_file(const char *path);

    bool loadBool(SaveDataRaw *d, const char *name, bool def);

    int loadInt(SaveDataRaw *d, const char *name, int def);

    float loadFloat(SaveDataRaw *d, const char *name, float def);

    char loadChar(SaveDataRaw *d, const char *name, char def);

    char *loadString(SaveDataRaw *d, const char *name, char *def);

    void load_close(SaveDataRaw *d);

#ifdef __cplusplus
}
#endif