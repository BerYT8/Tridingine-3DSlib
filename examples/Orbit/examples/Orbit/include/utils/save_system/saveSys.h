#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"

void save_init(
    SaveContext *ctx,
    bool bigEndian,
    uint16_t major,
    uint16_t minor,
    uint16_t sub);

void save_free(SaveContext *ctx);

void save_remove_value(SaveContext *ctx, const char *name);
void save_add_bool(SaveContext *ctx, const char *name, bool value);
void save_add_int32(SaveContext *ctx, const char *name, int32_t value);
void save_add_uint32(SaveContext *ctx, const char *name, uint32_t value);
void save_add_float(SaveContext *ctx, const char *name, float value);
void save_add_string(SaveContext *ctx, const char *name, const char *value);
void save_add_blob(SaveContext *ctx, const char *name, const void *data, uint32_t size);

bool save_write_file(SaveContext *ctx, const char *path);

#ifdef __cplusplus
}
#endif