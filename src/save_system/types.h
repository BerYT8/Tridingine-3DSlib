#pragma once

#define TYPE_INT 1
#define TYPE_FLOAT 2
#define TYPE_CHAR 3
#define TYPE_STRING 4
#define TYPE_BOOL 5

#include <stdbool.h>

typedef struct
{
    int vM;
    int vB;
    int vS;
} version;

#ifdef __cplusplus
extern "C"
{
#endif

    const char *getPath(const char* path, bool create);

    version getSaveVersion();

#ifdef __cplusplus
}
#endif