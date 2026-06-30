#include <utils/save_system/loadSys.h>

#include <stdlib.h>
#include <string.h>
#include "types.h"

/* ========================================================= */
/* ===================== ENDIAN ============================ */
/* ========================================================= */

static uint16_t read_u16(FILE *f, bool be)
{
    uint8_t b[2];

    if (fread(b, 1, 2, f) != 2)
        return 0;

    if (be)
    {
        return ((uint16_t)b[0] << 8) |
               (uint16_t)b[1];
    }

    return ((uint16_t)b[1] << 8) |
           (uint16_t)b[0];
}

static uint32_t read_u32(FILE *f, bool be)
{
    uint8_t b[4];

    if (fread(b, 1, 4, f) != 4)
        return 0;

    if (be)
    {
        return ((uint32_t)b[0] << 24) |
               ((uint32_t)b[1] << 16) |
               ((uint32_t)b[2] << 8) |
               (uint32_t)b[3];
    }

    return ((uint32_t)b[3] << 24) |
           ((uint32_t)b[2] << 16) |
           ((uint32_t)b[1] << 8) |
           (uint32_t)b[0];
}

/* ========================================================= */
/* ===================== INTERNAL ========================== */
/* ========================================================= */

static bool read_bit(
    const uint8_t *data,
    uint32_t bitOffset)
{
    uint32_t byte =
        bitOffset / 8;

    uint32_t bit =
        bitOffset % 8;

    return (data[byte] >> bit) & 1;
}

static bool validate_range(
    SaveFileData *file,
    SaveEntry *e)
{
    if (!file || !e)
        return false;

    uint32_t byteOffset =
        e->relativeBitOffset / 8;

    uint32_t byteSize =
        (e->dataSizeBits + 7) / 8;

    if (byteOffset > file->size)
        return false;

    if (byteOffset + byteSize > file->size)
        return false;

    return true;
}

/* ========================================================= */
/* ===================== FIND ENTRY ======================== */
/* ========================================================= */

SaveEntry *load_find_entry(
    SaveFileData *file,
    const char *name)
{
    if (!file || !name)
        return NULL;

    for (uint32_t i = 0; i < file->entryCount; i++)
    {
        if (strcmp(file->entries[i].name, name) == 0)
            return &file->entries[i];
    }

    return NULL;
}

/* ========================================================= */
/* ====================== LOAD FILE ======================== */
/* ========================================================= */

bool load_file(
    const char *path,
    SaveFileData *out)
{
    memset(out, 0, sizeof(*out));

    FILE *f = fopen(getPath(path, false), "rb");

    if (!f)
        return false;

    /* obtener tamaño total */

    fseek(f, 0, SEEK_END);

    long fileSize = ftell(f);

    rewind(f);

    if (fileSize <= 0)
    {
        fclose(f);
        return false;
    }

    /* flags */

    uint8_t flags = 0;

    if (fread(&flags, 1, 1, f) != 1)
    {
        fclose(f);
        return false;
    }

    bool be =
        (flags & SAVE_FLAG_BIG_ENDIAN) != 0;

    out->bigEndian = be;

    /* signature */

    char sig[5];

    memset(sig, 0, sizeof(sig));

    if (fread(sig, 1, 4, f) != 4)
    {
        fclose(f);
        return false;
    }

    if (strcmp(sig, SAVE_SIGNATURE) != 0)
    {
        fclose(f);
        return false;
    }

    /* version */

    out->version.major =
        read_u16(f, be);

    out->version.minor =
        read_u16(f, be);

    out->version.sub =
        read_u16(f, be);

    /* index size */

    uint32_t indexSize =
        read_u32(f, be);

    long indexStart =
        ftell(f);

    if (indexSize == 0)
    {
        fclose(f);
        return false;
    }

    /* read index */

    while ((uint32_t)(ftell(f) - indexStart) < indexSize)
    {
        long current =
            ftell(f) - indexStart;

        /* mínimo:
           2 nameLen
           1 type
           4 size
           4 offset
        */

        if ((uint32_t)current + 11 > indexSize)
            break;

        SaveEntry *newEntries =
            realloc(
                out->entries,
                sizeof(SaveEntry) *
                    (out->entryCount + 1));

        if (!newEntries)
        {
            fclose(f);
            return false;
        }

        out->entries = newEntries;

        SaveEntry *e =
            &out->entries[out->entryCount++];

        memset(e, 0, sizeof(*e));

        uint16_t nameLen =
            read_u16(f, be);

        if (nameLen == 0)
        {
            fclose(f);
            return false;
        }

        if ((uint32_t)(ftell(f) - indexStart) +
                nameLen + 9 >
            indexSize)
        {
            fclose(f);
            return false;
        }

        e->name =
            malloc(nameLen + 1);

        if (!e->name)
        {
            fclose(f);
            return false;
        }

        if (fread(e->name, 1, nameLen, f) != nameLen)
        {
            fclose(f);
            return false;
        }

        e->name[nameLen] = '\0';

        if (fread(&e->type, 1, 1, f) != 1)
        {
            fclose(f);
            return false;
        }

        e->dataSizeBits =
            read_u32(f, be);

        e->relativeBitOffset =
            read_u32(f, be);
    }

    long dataStart =
        ftell(f);

    long dataSize =
        fileSize - dataStart - 4;

    if (dataSize <= 0)
    {
        fclose(f);
        return false;
    }

    out->data =
        malloc((size_t)dataSize);

    if (!out->data)
    {
        fclose(f);
        return false;
    }

    out->size =
        (uint32_t)dataSize;

    if (fread(out->data, 1, dataSize, f) !=
        (size_t)dataSize)
    {
        fclose(f);
        free(out->data);
        return false;
    }

    fclose(f);

    return true;
}

/* ========================================================= */
/* ======================= GETTERS ========================= */
/* ========================================================= */

bool load_get_bool(
    SaveFileData *file,
    const char *name,
    bool def)
{
    SaveEntry *e =
        load_find_entry(file, name);

    if (!e)
        return def;

    if (!validate_range(file, e))
        return def;

    return read_bit(
        file->data,
        e->relativeBitOffset);
}

int32_t load_get_int32(
    SaveFileData *file,
    const char *name,
    int32_t def)
{
    SaveEntry *e =
        load_find_entry(file, name);

    if (!e)
        return def;

    if (!validate_range(file, e))
        return def;

    int32_t v;

    uint32_t byteOffset =
        e->relativeBitOffset / 8;

    memcpy(
        &v,
        file->data + byteOffset,
        sizeof(int32_t));

    return v;
}

uint32_t load_get_uint32(
    SaveFileData *file,
    const char *name,
    uint32_t def)
{
    SaveEntry *e =
        load_find_entry(file, name);

    if (!e)
        return def;

    if (!validate_range(file, e))
        return def;

    uint32_t v;

    uint32_t byteOffset =
        e->relativeBitOffset / 8;

    memcpy(
        &v,
        file->data + byteOffset,
        sizeof(uint32_t));

    return v;
}

float load_get_float(
    SaveFileData *file,
    const char *name,
    float def)
{
    SaveEntry *e =
        load_find_entry(file, name);

    if (!e)
        return def;

    if (!validate_range(file, e))
        return def;

    float v;

    uint32_t byteOffset =
        e->relativeBitOffset / 8;

    memcpy(
        &v,
        file->data + byteOffset,
        sizeof(float));

    return v;
}

double load_get_double(
    SaveFileData *file,
    const char *name,
    double def)
{
    SaveEntry *e =
        load_find_entry(file, name);

    if (!e)
        return def;

    if (!validate_range(file, e))
        return def;

    double v;

    uint32_t byteOffset =
        e->relativeBitOffset / 8;

    memcpy(
        &v,
        file->data + byteOffset,
        sizeof(double));

    return v;
}

const char *load_get_string(
    SaveFileData *file,
    const char *name,
    const char *def)
{
    SaveEntry *e =
        load_find_entry(file, name);

    if (!e)
        return def;

    if (!validate_range(file, e))
        return def;

    uint32_t byteOffset =
        e->relativeBitOffset / 8;

    return (const char *)(file->data + byteOffset);
}

void *load_get_blob(
    SaveFileData *file,
    const char *name,
    uint32_t *outSize)
{
    SaveEntry *e =
        load_find_entry(file, name);

    if (!e)
        return NULL;

    if (!validate_range(file, e))
        return NULL;

    uint32_t size =
        (e->dataSizeBits + 7) / 8;

    uint32_t byteOffset =
        e->relativeBitOffset / 8;

    void *buffer =
        malloc(size);

    if (!buffer)
        return NULL;

    memcpy(
        buffer,
        file->data + byteOffset,
        size);

    if (outSize)
        *outSize = size;

    return buffer;
}

/* ========================================================= */
/* ======================== FREE =========================== */
/* ========================================================= */

void load_free(SaveFileData *file)
{
    if (!file)
        return;

    for (uint32_t i = 0; i < file->entryCount; i++)
    {
        free(file->entries[i].name);
    }

    free(file->entries);

    free(file->data);

    memset(file, 0, sizeof(*file));
}