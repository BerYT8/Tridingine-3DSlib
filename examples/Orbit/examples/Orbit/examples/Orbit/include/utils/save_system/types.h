
#ifndef TYPES_SAVE_H
#define TYPES_SAVE_H
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define SAVE_SIGNATURE "SFTE"

#define SAVE_FLAG_VERSION_FIRST (1 << 0)
#define SAVE_FLAG_BIG_ENDIAN (1 << 1)
#define SAVE_FLAG_COMPRESSED (1 << 2)

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum SaveType
{
    SAVE_TYPE_BOOL = 0,
    SAVE_TYPE_INT8,
    SAVE_TYPE_INT16,
    SAVE_TYPE_INT32,
    SAVE_TYPE_INT64,
    SAVE_TYPE_UINT8,
    SAVE_TYPE_UINT16,
    SAVE_TYPE_UINT32,
    SAVE_TYPE_UINT64,
    SAVE_TYPE_FLOAT,
    SAVE_TYPE_DOUBLE,
    SAVE_TYPE_STRING,
    SAVE_TYPE_BLOB,
    SAVE_TYPE_ARRAY,
    SAVE_TYPE_OBJECT

} SaveType;

typedef struct nullValue
{
    int value;
} nullValue;

typedef struct SaveVersion
{
    uint16_t major;
    uint16_t minor;
    uint16_t sub;

} SaveVersion;

typedef struct SaveEntry
{
    char *name;

    uint8_t type;

    uint32_t dataSizeBits;

    uint32_t relativeBitOffset;

    void *data;

} SaveEntry;

typedef struct SaveContext
{
    SaveEntry *entries;

    uint32_t count;
    uint32_t capacity;

    bool bigEndian;

    SaveVersion version;

} SaveContext;

typedef struct BitWriter
{
    uint8_t *data;

    uint32_t capacityBytes;

    uint32_t bytePos;

    uint8_t bitPos;

} BitWriter;

typedef struct SaveFileData
{
    uint8_t *data;

    uint32_t size;

    SaveEntry *entries;

    uint32_t entryCount;

    bool bigEndian;

    SaveVersion version;

} SaveFileData;


#ifdef __cplusplus
}
#endif

#endif