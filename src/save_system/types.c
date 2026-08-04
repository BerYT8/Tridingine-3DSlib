#include "types.h"

#if defined(PLATFORM_PC)

    #if defined(_WIN32)

        #include <windows.h>
        #include <shlobj.h>

        #define PATH_SEP '\\'
        #define MKDIR(path) CreateDirectoryA(path, NULL)

    #elif defined(__linux__)

        #include <unistd.h>
        #include <limits.h>

        #define MAX_PATH PATH_MAX
        #define PATH_SEP '/'
        #define MKDIR(path) mkdir(path, 0755)

    #elif defined(__APPLE__)

        #include <mach-o/dyld.h>
        #include <limits.h>

        #define PATH_SEP '/'
        #define MKDIR(path) mkdir(path, 0755)

        #ifndef MAX_PATH
        #define MAX_PATH PATH_MAX
        #endif

    #endif
#elif defined(PLATFORM_3DS)
#include <3ds.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>

version getSaveVersion()
{
    version v;
    v.vM = 0;
    v.vB = 0;
    v.vS = 1;
    return v;
}

static const char* prefijx = "saves/";

#if defined(PLATFORM_PC)
static bool getExecutableDirectory(char* buffer, size_t size)
{
#if defined(_WIN32)

    if (!GetModuleFileNameA(NULL, buffer, size))
        return false;

    char* p = strrchr(buffer, '\\');
    if (p) *p = 0;

    return true;

#elif defined(__linux__)

    ssize_t len = readlink("/proc/self/exe", buffer, size - 1);
    if (len < 0)
        return false;

    buffer[len] = 0;

    char* p = strrchr(buffer, '/');
    if (p) *p = 0;

    return true;

#elif defined(__APPLE__)

    uint32_t s = (uint32_t)size;

    if (_NSGetExecutablePath(buffer, &s) != 0)
        return false;

    char* p = strrchr(buffer, '/');
    if (p) *p = 0;

    return true;

#else

    return false;

#endif
}
#elif defined(PLATFORM_3DS)
int mkdir_recursive(const char *path)
{
    char tmp[512];
    char *p = NULL;
    size_t len;
    struct stat st; // Estructura para verificar si el directorio ya existe

    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp) - 1] = '\0';

    len = strlen(tmp);
    if (len == 0)
        return -1;

    // Elimina '/' final
    if (tmp[len - 1] == '/')
        tmp[len - 1] = '\0';

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';

            // Si NO existe, intentamos crearlo. Si falla la creación, salimos.
            if (stat(tmp, &st) != 0) {
                if (mkdir(tmp, 0777) != 0)
                    return -1;
            }

            *p = '/';
        }
    }

    // Última verificación para el directorio final
    if (stat(tmp, &st) != 0) {
        if (mkdir(tmp, 0777) != 0)
            return -1;
    }

    return 0;
}
#endif

#if defined(PLATFORM_PC)
void crear_path_recursivo(const char *prefix, const char *ruta) {
    char buffer[MAX_PATH];

    snprintf(buffer, sizeof(buffer), "%s", prefix);

    const char *inicio = ruta;

    for (const char *p = ruta; ; p++) {

        if (*p == '/' || *p == '\\' || *p == '\0') {

            int len = p - inicio;

            if (len > 0) {
                // Inyectar el separador de carpetas correcto según el sistema operativo
                char sep_str[2] = { PATH_SEP, '\0' };
                strncat(buffer, sep_str, sizeof(buffer) - strlen(buffer) - 1);
                strncat(buffer, inicio, len);

                buffer[strlen(buffer)] = '\0';

#if defined(_WIN32)
                // Lógica exclusiva para Windows
                DWORD attrs = GetFileAttributesA(buffer);
                if (attrs == INVALID_FILE_ATTRIBUTES) {
                    CreateDirectoryA(buffer, NULL);
                }
#elif defined(__linux__) || defined(__APPLE__)
                // Lógica limpia y nativa para Linux (Arch Linux) y macOS
                struct stat st = {0};
                if (stat(buffer, &st) == -1) {
                    mkdir(buffer, 0755);
                }
#endif
            }

            if (*p == '\0') break;
            inicio = p + 1;
        }
    }
}
#endif

#define PATH_MAX_BUFFER 512 // Tamaño máximo seguro para cualquier ruta

const char* getPath(const char* path, bool create)
{
    // Búfer estático persistente: no necesita free() y es seguro de retornar
    static char finalPath[PATH_MAX_BUFFER];
    
    // Limpiamos el búfer antes de usarlo
    finalPath[0] = '\0';

#if defined(PLATFORM_PC)

    char exeDir[MAX_PATH];

    if (!getExecutableDirectory(exeDir, sizeof(exeDir)))
        return ""; // Ahora es seguro devolver "" porque nadie hará free()

    if (create)
        crear_path_recursivo(exeDir, prefijx);

    // Copia segura en el búfer estático
    snprintf(finalPath,
             sizeof(finalPath),
             "%s%c%s%c%s",
             exeDir,
             PATH_SEP,
             prefijx,
             PATH_SEP,
             path);

#if defined(_WIN32)

    for (char* p = finalPath; *p; ++p)
        if (*p == '/')
            *p = '\\';

#else

    for (char* p = finalPath; *p; ++p)
        if (*p == '\\')
            *p = '/';

#endif

    return finalPath;

#elif defined(PLATFORM_3DS)

    if (envIsHomebrew()) 
    {
        const char* in = "sdmc:/";

        // Copia segura en el búfer estático
        snprintf(finalPath,
                sizeof(finalPath),
                "%s%s%s",
                in,
                prefijx,
                path);

        // Opcional: Aquí puedes añadir mkdir() si 'create' es true para la 3DS
        mkdir_recursive(finalPath);

        return finalPath;
    }

    // Copia segura en el búfer estático
    snprintf(finalPath,
            sizeof(finalPath),
            "%s%s",
            prefijx,
            path);


    return finalPath;

#else

    // Si la plataforma no coincide, devuelve el path original de forma segura
    return path; 

#endif
}

// SVFILE API

struct SVFILE
{
#if defined(PLATFORM_3DS)
    bool useFsUser;

    union
    {
        FILE *stdio;

        struct
        {
            FS_Archive archive;
            Handle file;
            u64 offset;
        } fs;
    };
#else
    FILE *stdio;
#endif
};

SVFILE *svOpen(const char *path, const char *mode)
{
    SVFILE *f = calloc(1, sizeof(SVFILE));
    if (!f)
        return NULL;

#if !defined(PLATFORM_3DS)

    f->stdio = fopen(path, mode);
    if (!f->stdio)
    {
        free(f);
        return NULL;
    }

#else

    if (envIsHomebrew())
    {
        f->useFsUser = false;
        f->stdio = fopen(path, mode);

        if (!f->stdio)
        {
            free(f);
            return NULL;
        }
    }
    else
    {
        f->useFsUser = true;

        FS_Path archivePath = fsMakePath(PATH_EMPTY, "");

        if (R_FAILED(FSUSER_OpenArchive(&f->fs.archive,
                                        ARCHIVE_SDMC,
                                        archivePath)))
        {
            free(f);
            return NULL;
        }

        FS_Path filePath = fsMakePath(PATH_ASCII, path);

        u32 openFlags = 0;

        if (strchr(mode, 'r'))
            openFlags |= FS_OPEN_READ;

        if (strchr(mode, 'w'))
            openFlags |= FS_OPEN_WRITE | FS_OPEN_CREATE;

        if (strchr(mode, 'a'))
            openFlags |= FS_OPEN_WRITE | FS_OPEN_CREATE;

        if (R_FAILED(FSUSER_OpenFile(&f->fs.file,
                                     f->fs.archive,
                                     filePath,
                                     openFlags,
                                     0)))
        {
            FSUSER_CloseArchive(f->fs.archive);
            free(f);
            return NULL;
        }

        if (strchr(mode, 'a'))
        {
            u64 size;
            FSFILE_GetSize(f->fs.file, &size);
            f->fs.offset = size;
        }
        else
        {
            f->fs.offset = 0;
        }
    }

#endif

    return f;
}

size_t svRead(void *ptr, size_t size, size_t count, SVFILE *f)
{
#if defined(PLATFORM_PC)

    return fread(ptr, size, count, f->stdio);

#elif defined(PLATFORM_3DS)

    if (!f->useFsUser)
        return fread(ptr, size, count, f->stdio);

    u32 bytesRead = 0;
    u32 total = (u32)(size * count);

    if (R_FAILED(FSFILE_Read(f->fs.file,
                             &bytesRead,
                             f->fs.offset,
                             ptr,
                             total)))
        return 0;

    f->fs.offset += bytesRead;

    return bytesRead / size;

#endif
}

size_t svWrite(const void *ptr, size_t size, size_t count, SVFILE *f)
{
#if !defined(PLATFORM_3DS)

    return fwrite(ptr, size, count, f->stdio);

#else

    if (!f->useFsUser)
        return fwrite(ptr, size, count, f->stdio);

    u32 total = (u32)(size * count);

    if (R_FAILED(FSFILE_Write(f->fs.file,
                              NULL,
                              f->fs.offset,
                              ptr,
                              total,
                              FS_WRITE_FLUSH)))
        return 0;

    f->fs.offset += total;

    return count;

#endif
}

int svSeek(SVFILE *f, long offset, int origin)
{
#if !defined(PLATFORM_3DS)

    return fseek(f->stdio, offset, origin);

#else

    if (!f->useFsUser)
        return fseek(f->stdio, offset, origin);

    u64 newPos;

    switch (origin)
    {
        case SEEK_SET:
            newPos = offset;
            break;

        case SEEK_CUR:
            newPos = f->fs.offset + offset;
            break;

        case SEEK_END:
        {
            u64 size;
            FSFILE_GetSize(f->fs.file, &size);
            newPos = size + offset;
            break;
        }

        default:
            return -1;
    }

    f->fs.offset = newPos;
    return 0;

#endif
}

long svTell(SVFILE *f)
{
#if !defined(PLATFORM_3DS)

    return ftell(f->stdio);

#else

    if (!f->useFsUser)
        return ftell(f->stdio);

    return (long)f->fs.offset;

#endif
}

int svFlush(SVFILE *f)
{
#if !defined(PLATFORM_3DS)

    return fflush(f->stdio);

#else

    if (!f->useFsUser)
        return fflush(f->stdio);

    return FSFILE_Flush(f->fs.file);

#endif
}

int svClose(SVFILE *f)
{
    int ret = 0;

#if !defined(PLATFORM_3DS)

    ret = fclose(f->stdio);

#else

    if (!f->useFsUser)
    {
        ret = fclose(f->stdio);
    }
    else
    {
        FSFILE_Close(f->fs.file);
        FSUSER_CloseArchive(f->fs.archive);
    }

#endif

    free(f);

    return ret;
}

static inline uint16_t bswap16(uint16_t v)
{
    return (v >> 8) | (v << 8);
}

static inline uint32_t bswap32(uint32_t v)
{
    return ((v >> 24) & 0xff) |
           ((v >> 8)  & 0xff00) |
           ((v << 8)  & 0xff0000) |
           ((v << 24) & 0xff000000);
}

uint16_t svReadU16(SVFILE *f, bool be)
{
    uint16_t v;
    svRead(&v, sizeof(v), 1, f);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    if (be)
        v = bswap16(v);
#else
    if (!be)
        v = bswap16(v);
#endif

    return v;
}

uint32_t svReadU32(SVFILE *f, bool be)
{
    uint32_t v;
    svRead(&v, sizeof(v), 1, f);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    if (be)
        v = bswap32(v);
#else
    if (!be)
        v = bswap32(v);
#endif

    return v;
}

void svWriteU16(SVFILE *f, uint16_t v, bool be)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    if (be)
        v = bswap16(v);
#else
    if (!be)
        v = bswap16(v);
#endif

    svWrite(&v, sizeof(v), 1, f);
}

void svWriteU32(SVFILE *f, uint32_t v, bool be)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    if (be)
        v = bswap32(v);
#else
    if (!be)
        v = bswap32(v);
#endif

    svWrite(&v, sizeof(v), 1, f);
}