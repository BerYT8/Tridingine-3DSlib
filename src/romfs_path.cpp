#include "romfs_path.h"

#include <string>

const char* getRomfsPath(const char* path)
{
    static std::string p;

    p = path;

#if defined(PLATFORM_3DS)
    p = "romfs:/" + std::string(path);
#endif

    return p.c_str();
}