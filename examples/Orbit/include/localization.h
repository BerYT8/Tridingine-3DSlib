#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LOCALIZATION LOCALIZATION;

/*
==============================================================================
    Sistema de Localización
==============================================================================
*/

/* Inicializa el sistema */
LOCALIZATION *Loc_Init();

/* Libera toda la memoria usada por el sistema */
void Loc_Shutdown(LOCALIZATION *localization);

/* Carga un archivo de localización */
int Loc_LoadLocalization(LOCALIZATION *localization, const char* filePath);

int Loc_AddLocale(LOCALIZATION *localization, const char* filePath);
int Loc_LoadLocale(LOCALIZATION *localization, const char *locale);

bool Loc_CurrentLocaleIs(LOCALIZATION *localization, const char *locale);

/* Devuelve el locale del archivo de localización cargado */
const char *Loc_GetLocale(LOCALIZATION *localization);

/* Obtiene un texto traducido a partir de su nombre */
const char* Loc_GetText(LOCALIZATION *localization, const char* textName);

#ifdef __cplusplus
}
#endif