#pragma once

#if defined(PLATFORM_PC)

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
typedef struct __PAK_FILE __PAK_FILE;
typedef __PAK_FILE PAK_FILE;

#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#endif

void PAKL_SetPakFromMem(const void *data, size_t size);
void PAKL_SetPak(const char *path);

size_t PAKL_GetFileCount();
const char* PAKL_GetFileName(size_t index);

PAK_FILE *PAKL_LoadFile(const char *file);
size_t PAKL_GetFileSize(PAK_FILE* f);
long PAKL_ftell(PAK_FILE *f);
void PAKL_rewind(PAK_FILE *f);
size_t PAKL_fread(void *buff, size_t _size, size_t _n, PAK_FILE *f);
int PAKL_fseek(PAK_FILE *, long, int);
void PAKL_CloseFile(PAK_FILE *f);

void PAKL_ClosePak();

#ifdef __cplusplus
}
#endif

#elif defined(PLATFORM_3DS)

/* ===== 3DS: usa stdio normal ===== */
#include <stdio.h>

/* FILE estándar */
typedef FILE PAK_FILE;

/* Si no usas PAK en 3DS */
#define PAKL_SetPak(path)            ((void)0)

/* fopen/fclose wrapper */
#define PAKL_LoadFile(file)          fopen(file, "rb")
#define PAKL_CloseFile(f)            fclose(f)

#define PAKL_ftell(f)                ftell(f)
#define PAKL_rewind(f)               rewind(f)

#define PAKL_fread(b, s, n, f)       fread(b, s, n, f)
#define PAKL_fseek(f, o, w)          fseek(f, o, w)

#define PAKL_ClosePak()              ((void)0)

#endif