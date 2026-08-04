#include "romfs_path.h"

#include <string>

const char *getRomfsPath(const char *path)
{
    std::string in = "";
#if defined(PLATFORM_3DS)
    in = "romfs:/";
#endif
    return std::string(in + path).c_str();
}