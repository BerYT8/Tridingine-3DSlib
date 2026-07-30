#pragma once

#include <vector>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string.h>
#include <ints_defs.h>
#include <pak_loader/pak_loader.h>

// Esta estructura coincide exactamente con lo que hay escrito en el archivo
struct EntradaIndice {
    u32 hashNombre; // Ya viene calculado desde la PC
    u32 offsetTexto;// Posición relativa donde empieza el texto
};

struct localeEntry
{
    u32 hash;
    char *path;
};

constexpr u32 calcular_hash(const char* str) {
    u32 hash = 2166136261u;
    while (*str) {
        hash ^= static_cast<u32>(*str++);
        hash *= 16777619u;
    }
    return hash;
}

class Localization {
private:
    std::vector<char> archivoEnRAM; // Todo el archivo cargado en memoria de una sola vez
    const EntradaIndice* tablaIndice = nullptr;
    u16 totalElementos = 0;
    const char* bloqueTextosStart = nullptr;

    char locale[9] = "[ERROR]";

    std::vector<localeEntry> localesList = {};

public:
    bool cargarArchivo(const char* rutaArchivo) {

#if defined(PLATFORM_PC)
        // 1. Cargar el archivo desde el contenedor virtual PAK
        PAK_FILE* file = PAKL_LoadFile(rutaArchivo);
        if (!file) return false;

        // 2. Obtener el tamaño del archivo desde la propia estructura de tu PAK
        size_t tamArchivo = PAKL_GetFileSize(file);

        // 3. Volcar todo el archivo a la RAM de un solo viaje
        archivoEnRAM.resize(tamArchivo);
        PAKL_fread(archivoEnRAM.data(), 1, tamArchivo, file);
        
        // Cerramos el archivo virtual del PAK inmediatamente para liberar su memoria interna
        PAKL_CloseFile(file);
#elif defined(PLATFORM_3DS)
        FILE* file = fopen(rutaArchivo, "rb");
        if (!file) return false;

        // 1. Obtener tamaño del archivo y leerlo entero a la RAM de un solo viaje
        fseek(file, 0, SEEK_END);
        size_t tamArchivo = ftell(file);
        fseek(file, 0, SEEK_SET);

        archivoEnRAM.resize(tamArchivo);
        fread(archivoEnRAM.data(), 1, tamArchivo, file);
        fclose(file);
#endif

        // 4. Mapear la cabecera (Punteros directos a la RAM, ¡coste de copia cero!)
        char* ptr = archivoEnRAM.data();
        
        // Validar cabecera "LNGTE"
        if (std::string_view(ptr, 5) != "LNGTE") return false;
        ptr += 5; // Saltar "LNGTE"

        memcpy(locale, ptr, 8);
        locale[8] = '\0';

        printf("[DEBUG OF LOAD]: Locale: %s.\n", locale);
        
        ptr += 8; // Saltar idioma
        
        // Leer la cantidad de elementos (2 bytes)
        totalElementos = *reinterpret_cast<u16*>(ptr);
        ptr += 2;

        // El puntero ahora apunta exactamente donde empieza el Bloque 1 (Índice)
        tablaIndice = reinterpret_cast<const EntradaIndice*>(ptr);

        // Calcular dónde empieza el Bloque 2 (Textos)
        bloqueTextosStart = ptr + (totalElementos * sizeof(EntradaIndice));

        return true;
    }

    void restartList()
    {
        for(auto &l : localesList)
        {
            free(l.path);
        }
        localesList.clear();
    }

    void liberarTodo()
    {
        // 1. Liberar el buffer completo del archivo
        archivoEnRAM.clear();
        archivoEnRAM.shrink_to_fit();

        // 2. Invalidar punteros que apuntaban dentro del buffer
        tablaIndice = nullptr;
        bloqueTextosStart = nullptr;

        // 3. Reset de datos de estado
        totalElementos = 0;

        // 4. Reset locale
        strcpy(locale, "[ERROR]"); 
    }

    bool obtenerLocaleDeArchivo(const char* rutaArchivo, char localeOut[9])
    {
#if defined(PLATFORM_PC)
        
        // 1. Abrir el archivo virtual desde tu paquete binario
        PAK_FILE* file = PAKL_LoadFile(rutaArchivo);
        if (!file) {
            printf("[ERROR] No se pudo encontrar el archivo en el paquete PAK: %s\n", rutaArchivo);
            return false;
        }

        char cabecera[13]; // 5 bytes "LNGTE" + 8 bytes locale

        // 2. Leer únicamente los 13 bytes de la cabecera usando tu API
        if (PAKL_fread(cabecera, 1, sizeof(cabecera), file) != sizeof(cabecera))
        {
            printf("[ERROR] Lectura incompleta de cabecera en: %s\n", rutaArchivo);
            PAKL_CloseFile(file);
            return false;
        }

        // Cerramos el archivo virtual inmediatamente tras leer los bytes necesarios
        PAKL_CloseFile(file);

        // 3. Validar la firma "LNGTE"
        if (std::memcmp(cabecera, "LNGTE", 5) != 0) {
            printf("[ERROR] Cabecera invalida en archivo %s (Se esperaba LNGTE)\n", rutaArchivo);
            return false;
        }

        // 4. Extraer el locale de forma segura
        std::memcpy(localeOut, cabecera + 5, 8);
        localeOut[8] = '\0';

        return true;
#elif defined(PLATFORM_3DS)
        FILE* file = fopen(rutaArchivo, "rb");
        if (!file) {
            printf("[ERROR] No se pudo abrir el archivo (ruta incorrecta o no existe): %s\n", rutaArchivo);
            return false;
        }

        char cabecera[13]; // 5 bytes "LNGTE" + 8 bytes locale

        if (fread(cabecera, 1, sizeof(cabecera), file) != sizeof(cabecera))
        {
            printf("[ERROR] Lectura incompleta de cabecera en: %s\n", rutaArchivo);
            fclose(file);
            return false;
        }

        fclose(file);

        if (memcmp(cabecera, "LNGTE", 5) != 0) {
            printf("[ERROR] Cabecera invalida en archivo %s (Se esperaba LNGTE)\n", rutaArchivo);
            return false;
        }

        memcpy(localeOut, cabecera + 5, 8);
        localeOut[8] = '\0';

        return true;
#endif
    }


    void addLocaleToList(u32 hashEntrada, const char* path)
    {
        char *b = (char*)malloc(strlen(path) + 1);  // +1 para el '\0'
        strcpy(b, path);
        localesList.emplace_back(hashEntrada, b);
        ordenarLista();
    }

    int loadFromLocale(const char* locale)
    {
        u32 hashBuscado = calcular_hash(locale); 

        localeEntry valorBuscado{};
        valorBuscado.hash = hashBuscado;

        auto it = std::lower_bound(
            localesList.begin(),
            localesList.end(),
            valorBuscado,
            [](const localeEntry& a, const localeEntry& b)
            {
                return a.hash < b.hash;
            });

        if (it != localesList.end()) {
            if (it->hash == hashBuscado) {
                bool exitoCarga = cargarArchivo(it->path);
                return exitoCarga ? 1 : 0;
            }
        } else {
            printf("[ERROR] lower_bound llego al final de la lista. Elemento no encontrado.\n");
        }

        return 0;
    }

    void ordenarLista() {
        std::sort(localesList.begin(), localesList.end(), [](const localeEntry& a, const localeEntry& b) {
            return a.hash < b.hash;
        });
    }

    const char* obtenerLocale()
    {
        if (totalElementos == 0 || tablaIndice == nullptr) {
            return "[ERROR: LOCALE_NOT_FOUND]";
        }
        return locale;
    }

    // BÚSQUEDA BINARIA ULTRA RÁPIDA (O(log N))
    const char* obtenerTexto(u32 hashBuscado) const {
        if (totalElementos == 0 || tablaIndice == nullptr) {
            return "[ERROR: TEXT_NOT_FOUND]";
        }

        // Creamos un objeto temporal con el hash que queremos buscar
        EntradaIndice valorBuscado;
        valorBuscado.hashNombre = hashBuscado;
        valorBuscado.offsetTexto = 0;

        const EntradaIndice *end = tablaIndice + static_cast<size_t>(totalElementos);

        // Ahora comparamos dos estructuras del mismo tipo, eliminando el error del compilador
        auto it = std::lower_bound(
            tablaIndice,
            end,
            valorBuscado,
            [](const EntradaIndice& a, const EntradaIndice& b)
            {
                return a.hashNombre < b.hashNombre;
            });

        if (it != (tablaIndice + totalElementos) && it->hashNombre == hashBuscado) {
            // El offset nos dice cuántos bytes avanzar desde el inicio del bloque de textos
            return bloqueTextosStart + it->offsetTexto;
        }
        
        return "[ERROR: TEXT_NOT_FOUND]";
    }
};