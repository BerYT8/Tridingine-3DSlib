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

#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

                strncat(buffer, "\\", sizeof(buffer) - strlen(buffer) - 1);
                strncat(buffer, inicio, len);

                buffer[strlen(buffer)] = '\0';

                DWORD attrs = GetFileAttributesA(buffer);

                if (attrs == INVALID_FILE_ATTRIBUTES) {
                    DWORD err = GetLastError();

                    CreateDirectoryA(buffer, NULL);

                }
            }

            if (*p == '\0') break;
            inicio = p + 1;
        }
    }
}
#endif

const char* getPath(const char* path, bool create)
{
#if defined(PLATFORM_PC)

    char exeDir[MAX_PATH];

    if (!getExecutableDirectory(exeDir, sizeof(exeDir)))
        return "";

    if (create)
        crear_path_recursivo(exeDir, prefijx);

    int total = strlen(exeDir) + strlen(prefijx) + strlen(path) + 3;

    char* final = (char*)malloc(total);

    if (!final)
        return "";

    snprintf(final,
             total,
             "%s%c%s%c%s",
             exeDir,
             PATH_SEP,
             prefijx,
             PATH_SEP,
             path);

#if defined(_WIN32)

    for (char* p = final; *p; ++p)
        if (*p == '/')
            *p = '\\';

#else

    for (char* p = final; *p; ++p)
        if (*p == '\\')
            *p = '/';

#endif

    return final;

#else

    return path;

#endif
}