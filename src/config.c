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

extern const char* TRIDINGINE_GAME_NAME_TEXT;
extern const char* TRIDINGINE_GAME_DESCRIPTION_TEXT;
extern const char* TRIDINGINE_GAME_AUTHOR_TEXT;

const char* getGameName()
{
    return TRIDINGINE_GAME_NAME_TEXT;
}

const char* getGameDescription()
{
    return TRIDINGINE_GAME_DESCRIPTION_TEXT;
}

const char* getGameAuthor()
{
    return TRIDINGINE_GAME_AUTHOR_TEXT;
}

Vec3 getVersion()
{
    return vec3_create(TRIDINGINE_VERSION_MAJOR, TRIDINGINE_VERSION_MINOR, TRIDINGINE_VERSION_MICRO);
}