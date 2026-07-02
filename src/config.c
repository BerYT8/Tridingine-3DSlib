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

const char *
getAppUniqueID()
{
    return "3DSLIBS";
}

const char *getAppName()
{
    return "3DSLibs";
}

const char *getAppVersion()
{
    return "0.1.0";
}

const char *getAppAuthor()
{
    return "Ber";
}

const char *getAppDescription()
{
    return "A collection of libraries for 3DS homebrew development.";
}
