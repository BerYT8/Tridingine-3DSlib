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

    bool isAppCia();

#ifdef __cplusplus
}
#endif