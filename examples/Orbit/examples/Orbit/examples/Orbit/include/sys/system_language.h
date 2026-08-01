#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void System_SetCurrentLang(const char *lang);
const char *System_GetCurrentLang();

#ifdef __cplusplus
}
#endif