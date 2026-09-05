#include <Tridingine.h>
#include "config.h"

static AppType appType = TDSX;

void setAppType(AppType type)
{
    appType = type;
}

bool isAppCia()
{
    return appType == CIA;
}

Vec3 getVersion()
{
    return vec3_create(TRIDINGINE_VERSION_MAJOR, TRIDINGINE_VERSION_MINOR, TRIDINGINE_VERSION_MICRO);
}