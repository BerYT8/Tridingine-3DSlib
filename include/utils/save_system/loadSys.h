#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"

bool load_file(const char *path, SaveFileData *out);

void load_free(SaveFileData *file);

bool load_get_bool(SaveFileData *file, const char *name, bool def);
int32_t load_get_int32(SaveFileData *file, const char *name, int32_t def);
uint32_t load_get_uint32(SaveFileData *file, const char *name, uint32_t def);
float load_get_float(SaveFileData *file, const char *name, float def);
const char *load_get_string(SaveFileData *file, const char *name, const char *def);

SaveEntry *load_find_entry(SaveFileData *file, const char *name);

#ifdef __cplusplus
}
#endif