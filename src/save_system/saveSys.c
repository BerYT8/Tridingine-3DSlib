#include <utils/save_system/saveSys.h>

#include <stdlib.h>
#include <string.h>
#include "types.h"

/* ========================================================= */
/* ====================== CRC32 ============================ */
/* ========================================================= */

static uint32_t crc32_table[256];

static void crc32_init()
{
    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t c = i;

        for (int j = 0; j < 8; j++)
        {
            if (c & 1)
                c = 0xEDB88320 ^ (c >> 1);
            else
                c >>= 1;
        }

        crc32_table[i] = c;
    }
}

static uint32_t crc32_compute(
    const uint8_t *data,
    size_t size)
{
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < size; i++)
    {
        crc =
            crc32_table[(crc ^ data[i]) & 0xFF] ^
            (crc >> 8);
    }

    return crc ^ 0xFFFFFFFF;
}

/* ========================================================= */
/* ===================== ENDIAN ============================ */
/* ========================================================= */

static void write_u16(
    FILE *f,
    uint16_t v,
    bool be)
{
    uint8_t b[2];

    if (be)
    {
        b[0] = (v >> 8) & 0xFF;
        b[1] = v & 0xFF;
    }
    else
    {
        b[1] = (v >> 8) & 0xFF;
        b[0] = v & 0xFF;
    }

    fwrite(b, 1, 2, f);
}

static void write_u32(
    FILE *f,
    uint32_t v,
    bool be)
{
    uint8_t b[4];

    if (be)
    {
        b[0] = (v >> 24) & 0xFF;
        b[1] = (v >> 16) & 0xFF;
        b[2] = (v >> 8) & 0xFF;
        b[3] = v & 0xFF;
    }
    else
    {
        b[3] = (v >> 24) & 0xFF;
        b[2] = (v >> 16) & 0xFF;
        b[1] = (v >> 8) & 0xFF;
        b[0] = v & 0xFF;
    }

    fwrite(b, 1, 4, f);
}

/* ========================================================= */
/* ===================== BIT WRITER ======================== */
/* ========================================================= */

static void bitwriter_init(
    BitWriter *bw,
    uint32_t capacity)
{
    bw->data =
        calloc(1, capacity);

    bw->capacityBytes =
        capacity;

    bw->bytePos = 0;
    bw->bitPos = 0;
}

static void bitwriter_write_bit(
    BitWriter *bw,
    bool bit)
{
    if (bit)
    {
        bw->data[bw->bytePos] |=
            (1 << bw->bitPos);
    }

    bw->bitPos++;

    if (bw->bitPos >= 8)
    {
        bw->bitPos = 0;
        bw->bytePos++;
    }
}

static void bitwriter_write_byte(
    BitWriter *bw,
    uint8_t byte)
{
    if (bw->bitPos == 0)
    {
        bw->data[bw->bytePos++] = byte;
        return;
    }

    for (int i = 0; i < 8; i++)
    {
        bool bit =
            (byte >> i) & 1;

        bitwriter_write_bit(
            bw,
            bit);
    }
}

static uint32_t bitwriter_total_bits(
    BitWriter *bw)
{
    return (bw->bytePos * 8) +
           bw->bitPos;
}

/* ========================================================= */
/* ================= SAVE INTERNAL ========================= */
/* ========================================================= */

static void save_reserve(
    SaveContext *ctx)
{
    if (ctx->count < ctx->capacity)
        return;

    ctx->capacity =
        (ctx->capacity == 0)
            ? 8
            : ctx->capacity * 2;

    SaveEntry *newEntries =
        realloc(
            ctx->entries,
            sizeof(SaveEntry) *
                ctx->capacity);

    if (!newEntries)
        return;

    ctx->entries = newEntries;
}

static void save_add_raw(
    SaveContext *ctx,
    const char *name,
    uint8_t type,
    const void *data,
    uint32_t sizeBits)
{
    save_remove_value(ctx, name);

    save_reserve(ctx);

    SaveEntry *e =
        &ctx->entries[ctx->count++];

    memset(e, 0, sizeof(*e));

    e->name = strdup(name);

    e->type = type;

    e->dataSizeBits =
        sizeBits;

    uint32_t bytes =
        (sizeBits + 7) / 8;

    e->data =
        malloc(bytes);

    memcpy(
        e->data,
        data,
        bytes);
}

/* ========================================================= */
/* ======================= PUBLIC ========================== */
/* ========================================================= */

void save_init(
    SaveContext *ctx,
    bool bigEndian,
    uint16_t major,
    uint16_t minor,
    uint16_t sub)
{
    memset(ctx, 0, sizeof(*ctx));

    ctx->bigEndian =
        bigEndian;

    ctx->version.major =
        major;

    ctx->version.minor =
        minor;

    ctx->version.sub =
        sub;

    crc32_init();
}

void save_free(
    SaveContext *ctx)
{
    for (uint32_t i = 0; i < ctx->count; i++)
    {
        free(ctx->entries[i].name);
        free(ctx->entries[i].data);
    }

    free(ctx->entries);

    memset(ctx, 0, sizeof(*ctx));
}

void save_remove_value(
    SaveContext *ctx,
    const char *name)
{
    if (!ctx || !name)
        return;

    for (uint32_t i = 0; i < ctx->count; i++)
    {
        SaveEntry *e =
            &ctx->entries[i];

        if (strcmp(e->name, name) == 0)
        {
            free(e->name);
            free(e->data);

            for (uint32_t j = i + 1;
                 j < ctx->count;
                 j++)
            {
                ctx->entries[j - 1] =
                    ctx->entries[j];
            }

            ctx->count--;

            return;
        }
    }
}

void save_add_bool(
    SaveContext *ctx,
    const char *name,
    bool value)
{
    uint8_t v =
        value ? 1 : 0;

    save_add_raw(
        ctx,
        name,
        SAVE_TYPE_BOOL,
        &v,
        1);
}

void save_add_int32(
    SaveContext *ctx,
    const char *name,
    int32_t value)
{
    save_add_raw(
        ctx,
        name,
        SAVE_TYPE_INT32,
        &value,
        32);
}

void save_add_uint32(
    SaveContext *ctx,
    const char *name,
    uint32_t value)
{
    save_add_raw(
        ctx,
        name,
        SAVE_TYPE_UINT32,
        &value,
        32);
}

void save_add_float(
    SaveContext *ctx,
    const char *name,
    float value)
{
    save_add_raw(
        ctx,
        name,
        SAVE_TYPE_FLOAT,
        &value,
        32);
}

void save_add_double(
    SaveContext *ctx,
    const char *name,
    double value)
{
    save_add_raw(
        ctx,
        name,
        SAVE_TYPE_DOUBLE,
        &value,
        64);
}

void save_add_string(
    SaveContext *ctx,
    const char *name,
    const char *value)
{
    uint32_t bits =
        (strlen(value) + 1) * 8;

    save_add_raw(
        ctx,
        name,
        SAVE_TYPE_STRING,
        value,
        bits);
}

void save_add_blob(
    SaveContext *ctx,
    const char *name,
    const void *data,
    uint32_t size)
{
    save_add_raw(
        ctx,
        name,
        SAVE_TYPE_BLOB,
        data,
        size * 8);
}

static uint32_t calculate_index_size(
    SaveContext *ctx)
{
    uint32_t total = 0;

    for (uint32_t i = 0; i < ctx->count; i++)
    {
        SaveEntry *e =
            &ctx->entries[i];

        total += 2;
        total += strlen(e->name);
        total += 1;
        total += 4;
        total += 4;
    }

    return total;
}

bool save_write_file(
    SaveContext *ctx,
    const char *path)
{
    FILE *f =
        fopen(getPath(path, true), "wb+");

    if (!f)
        return false;

    uint32_t estimatedSize = 64;

    for (uint32_t i = 0; i < ctx->count; i++)
    {
        estimatedSize +=
            (ctx->entries[i].dataSizeBits + 7) / 8;
    }

    BitWriter bw;

    bitwriter_init(
        &bw,
        estimatedSize);

    for (uint32_t i = 0; i < ctx->count; i++)
    {
        SaveEntry *e =
            &ctx->entries[i];

        e->relativeBitOffset =
            bitwriter_total_bits(&bw);

        uint32_t bytes =
            (e->dataSizeBits + 7) / 8;

        if (e->type == SAVE_TYPE_BOOL)
        {
            uint8_t b =
                *((uint8_t *)e->data);

            bitwriter_write_bit(
                &bw,
                b != 0);
        }
        else
        {
            uint8_t *ptr =
                e->data;

            for (uint32_t j = 0;
                 j < bytes;
                 j++)
            {
                bitwriter_write_byte(
                    &bw,
                    ptr[j]);
            }
        }
    }

    uint32_t indexSize =
        calculate_index_size(ctx);

    uint8_t flags = 0;

    if (ctx->bigEndian)
    {
        flags |= SAVE_FLAG_BIG_ENDIAN;
    }

    fwrite(&flags, 1, 1, f);

    fwrite(
        SAVE_SIGNATURE,
        1,
        4,
        f);

    write_u16(
        f,
        ctx->version.major,
        ctx->bigEndian);

    write_u16(
        f,
        ctx->version.minor,
        ctx->bigEndian);

    write_u16(
        f,
        ctx->version.sub,
        ctx->bigEndian);

    write_u32(
        f,
        indexSize,
        ctx->bigEndian);

    for (uint32_t i = 0; i < ctx->count; i++)
    {
        SaveEntry *e =
            &ctx->entries[i];

        uint16_t nameLen =
            strlen(e->name);

        write_u16(
            f,
            nameLen,
            ctx->bigEndian);

        fwrite(
            e->name,
            1,
            nameLen,
            f);

        fwrite(
            &e->type,
            1,
            1,
            f);

        write_u32(
            f,
            e->dataSizeBits,
            ctx->bigEndian);

        write_u32(
            f,
            e->relativeBitOffset,
            ctx->bigEndian);
    }

    uint32_t dataBytes =
        (bitwriter_total_bits(&bw) + 7) / 8;

    fwrite(
        bw.data,
        1,
        dataBytes,
        f);

    fflush(f);

    long endPos =
        ftell(f);

    rewind(f);

    uint8_t *allData =
        malloc(endPos);

    if (!allData)
    {
        free(bw.data);
        fclose(f);
        return false;
    }

    if (fread(allData, 1, endPos, f) !=
        (size_t)endPos)
    {
        free(allData);
        free(bw.data);
        fclose(f);
        return false;
    }

    uint32_t crc =
        crc32_compute(
            allData,
            endPos);

    free(allData);

    fseek(f, 0, SEEK_END);

    write_u32(
        f,
        crc,
        ctx->bigEndian);

    free(bw.data);

    fclose(f);

    return true;
}