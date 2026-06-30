#pragma once

#include "canvas/canvas.h"

#include <stdbool.h>

typedef struct UI_Element_s* UI_Element;

#ifdef __cplusplus
extern "C" {
#endif

bool UI_AssignPartner(UI_Element element, UI_Element partner);

void UI_DeleteElement(UI_Element element);

#ifdef __cplusplus
}
#endif