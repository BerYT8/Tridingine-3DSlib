#pragma once

#include <stdbool.h>

typedef enum
{
    CIA,
    TDSX
} AppType;

#ifdef __cplusplus
extern "C"
{
#endif
    void setAppType(AppType type);

    const char *getAppName();
    const char *getAppVersion();
    const char *getAppAuthor();
    const char *getAppDescription();
    const char *getAppUniqueID();

    bool isAppCia();

#ifdef __cplusplus
}
#endif