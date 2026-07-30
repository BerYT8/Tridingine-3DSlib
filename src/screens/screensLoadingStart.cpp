#if defined(PLATFORM_PC)

#include <utils/save_system.h>
#include "screensLoadingStart.h"
#include <cstring> // 👈 SOLUCIÓN: Necesario para strdup, strcpy y funciones de memoria de C

SaveForCArray *loadInitial()
{
    AsyncSaveData loaded;
    loadSimple("windows.info", loaded);

    if(!loaded.getBoolValue("fullscreen", false))
        loaded.addValue("fullscreen", false);

    saveSimple("windows.info", loaded);

    std::vector<AsyncValue> list = loaded.getValuesList();

    // 1. Usar new para asegurar que se construya la estructura correctamente
    SaveForCArray *array = new SaveForCArray();
    array->size = list.size();

    // 2. Si SaveForC contiene tipos C++, DEBES usar new[] en vez de malloc
    array->list = new SaveForC[array->size];

    for(unsigned int i = 0; i < array->size; i++)
    {
        // Asignación segura del string (c_str() si name es std::string)
        array->list[i].name = strdup(list[i].name.c_str()); // 👈 Ahora compila gracias a <cstring>
        
        unsigned int size = 0;
        int type = -1;
        
        if (list[i].value.isBool()) 
        {
            array->list[i].data = malloc(sizeof(bool));
            *static_cast<bool*>(array->list[i].data) = list[i].value.as<bool>();
            type = SAVE_TYPE_BOOL;
        }
        else if (list[i].value.isInt()) 
        {
            array->list[i].data = malloc(sizeof(int32_t));
            *static_cast<int32_t*>(array->list[i].data) = list[i].value.as<int32_t>();
            type = SAVE_TYPE_INT32;
        }
        else if (list[i].value.isUInt()) 
        {
            array->list[i].data = malloc(sizeof(uint32_t));
            *static_cast<uint32_t*>(array->list[i].data) = list[i].value.as<uint32_t>();
            type = SAVE_TYPE_UINT32;
        }
        else if (list[i].value.isFloat()) 
        {
            array->list[i].data = malloc(sizeof(float));
            *static_cast<float*>(array->list[i].data) = list[i].value.as<float>();
            type = SAVE_TYPE_FLOAT;
        }
        else if (list[i].value.isDouble()) 
        {
            array->list[i].data = malloc(sizeof(double));
            *static_cast<double*>(array->list[i].data) = list[i].value.as<double>();
            type = SAVE_TYPE_DOUBLE;
        }
        else if (list[i].value.isString()) 
        {
            std::string str_val = list[i].value.as<std::string>();
            char* c_str = static_cast<char*>(malloc(str_val.length() + 1));
            strcpy(c_str, str_val.c_str()); // 👈 SOLUCIÓN: Cambiado std::strcpy por strcpy estándar de C
            array->list[i].data = c_str; 
            type = SAVE_TYPE_STRING;
        }
        
        array->list[i].type = type;
        
        // CORRECCIÓN: .c_str() para el %s de printf. 
        // Nota: El último parámetro imprimirá 1 o 0 porque asumes bool. Elige un casteo genérico si varía.
        printf("[ITEM] Name: %s, Type: %d, value: %d.\n", 
            list[i].name.c_str(), 
            type, 
            list[i].value.isBool() ? list[i].value.as<bool>() : 0);
    }

    return array;
}

void setScreenValue(const char* name, void* value, SaveType type)
{
    AV v; // Aquí construiremos el variant correcto

    switch (type) {
        case SAVE_TYPE_BOOL:
            v = *(bool*)value;
            break;
        case SAVE_TYPE_INT32:
            v = *(int32_t*)value; // Cassea correctamente al int original
            break;
        case SAVE_TYPE_UINT32:
            v = *(uint32_t*)value;
            break;
        case SAVE_TYPE_FLOAT:
            v = *(float*)value;
            break;
        case SAVE_TYPE_DOUBLE:
            v = *(double*)value;
            break;
        case SAVE_TYPE_STRING:
            // C maneja "char*", lo convertimos a std::string de C++
            v = std::string((char*)value);
            break;
        default:
            v = nullValue();
            break;
    }

    AsyncSaveData loaded;
    loadSimple("windows.info", loaded);

    loaded.addValue(name, v);

    saveSimple("windows.info", loaded);
}

void freeLoadedList(SaveForCArray *list)
{
    for(int i = 0; i < list->size; i++)
    {
        if(!list->list[i].data || !list->list[i].name)
            continue;
        free(list->list[i].data);
    }
    free(list->list);
    free(list);
}

#endif
