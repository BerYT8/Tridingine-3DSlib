#include <ui/user_interface.h>
#include "definitions.h"

#include <stdlib.h>

bool UI_AssignPartner(UI_Element element, UI_Element partner)
{
    if(!element || !partner)
        return false;

    element->assigned = partner;

    if(!element->assigned)
        return false;

    return true;
}

void UI_DeleteElement(UI_Element element)
{
    free(element);
}