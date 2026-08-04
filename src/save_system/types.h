#pragma once

#define TYPE_INT 1
#define TYPE_FLOAT 2
#define TYPE_CHAR 3
#define TYPE_STRING 4
#define TYPE_BOOL 5

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct SVFILE SVFILE;

SVFILE *svOpen(const char *path, const char *mode);
size_t  svRead(void *ptr, size_t size, size_t count, SVFILE *f);
size_t  svWrite(const void *ptr, size_t size, size_t count, SVFILE *f);
int     svSeek(SVFILE *f, long offset, int origin);
long    svTell(SVFILE *f);
int     svFlush(SVFILE *f);
int     svClose(SVFILE *f);

uint16_t svReadU16(SVFILE *f, bool be);
uint32_t svReadU32(SVFILE *f, bool be);
void     svWriteU16(SVFILE *f, uint16_t v, bool be);
void     svWriteU32(SVFILE *f, uint32_t v, bool be);