#include <localization.h>
#include "localization_sys.h"

typedef struct LOCALIZATION {
    Localization *content;
} LOCALIZATION;

LOCALIZATION *Loc_Init(void)
{
    LOCALIZATION *loc = new LOCALIZATION();
    loc->content = new Localization();
    if(!loc->content)
    {
        delete loc;
        return NULL;
    }
    return loc;
}

void Loc_Shutdown(LOCALIZATION *localization)
{
    if(!localization)
        return;
    if(localization->content)
    {
        localization->content->liberarTodo();
        delete localization->content;
    }
    delete localization;
}

int Loc_LoadLocalization(LOCALIZATION *localization, const char* filePath)
{
    if(localization && localization->content)
    {
        char p[512];
#if defined(PLATFORM_PC)
        snprintf(p, sizeof(p), "%s", filePath);
#elif defined(PLATFORM_3DS)
        snprintf(p, sizeof(p), "romfs:/%s", filePath);
#endif
        return localization->content->cargarArchivo(p) ? 1 : 0;
    }
    return 0;
}


int Loc_AddLocale(LOCALIZATION *localization, const char* filePath)
{
    if(!localization)
        return 0;
    char localeOut[9] = {};
    char p[512];
#if defined(PLATFORM_PC)
    snprintf(p, sizeof(p), "%s", filePath);
#elif defined(PLATFORM_3DS)
    snprintf(p, sizeof(p), "romfs:/%s", filePath);
#endif
    bool c = localization->content->obtenerLocaleDeArchivo(p, localeOut);
    if(!c)
        return 0;
    localization->content->addLocaleToList(calcular_hash(localeOut), p);
    return 1;
}

int Loc_LoadLocale(LOCALIZATION *localization, const char *locale)
{
    if(!localization)
        return 0;
    return localization->content->loadFromLocale(locale);
}

bool Loc_CurrentLocaleIs(LOCALIZATION *localization, const char *locale){
    if(!localization)
        return false;
    return strcmp(Loc_GetLocale(localization), locale) == 0 ? true : false;
}

const char *Loc_GetLocale(LOCALIZATION *localization)
{
    if(localization)
        return localization->content->obtenerLocale();
    return "[ERROR: LOCALIZATION_NOT_FOUND]";
}

const char* Loc_GetText(LOCALIZATION *localization, const char* textName)
{
    if(localization && localization->content)
        return localization->content->obtenerTexto(calcular_hash(textName));

    return "[ERROR: LOCALIZATION_NOT_FOUND]";
}