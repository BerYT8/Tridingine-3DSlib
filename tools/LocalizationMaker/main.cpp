#include <iostream>
#include <vector>
#include <string.h>
#include <cstring>
#include <cstdint>
#include <fstream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

#include "json.hpp"

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

using json = nlohmann::json;

struct translation {
    u32 hashName;
    std::string text;
};

struct Archivo {
    u32 hashLocale;
    std::vector<translation> translations;
};

struct EntradaIndiceBinario {
    u32 hashName;
    u32 offsetTexto;
};

// FNV-1a hash
constexpr u32 calcular_hash(const char* str) {
    u32 hash = 2166136261u;
    while (*str) {
        hash ^= static_cast<u32>(*str++);
        hash *= 16777619u;
    }
    return hash;
}

void exportarArchivoLang(const Archivo& datos, const std::string& codigoIdioma, const std::string& rutaSalida) {

    auto traduccionesOrdenadas = datos.translations;
    std::sort(traduccionesOrdenadas.begin(), traduccionesOrdenadas.end(),
        [](const translation& a, const translation& b) {
            return a.hashName < b.hashName;
        });

    std::ofstream archivo(rutaSalida, std::ios::binary);
    if (!archivo.is_open()) {
        std::cerr << "Error al crear el archivo de salida: " << rutaSalida << "\n";
        return;
    }

    // CABECERA
    archivo.write("LNGTE", 5);

    char idiomaBuffer[8] = {0};
    std::strncpy(idiomaBuffer, codigoIdioma.c_str(), sizeof(idiomaBuffer) - 1);
    archivo.write(idiomaBuffer, 8);

    u16 totalElementos = static_cast<u16>(traduccionesOrdenadas.size());
    archivo.write(reinterpret_cast<const char*>(&totalElementos), sizeof(totalElementos));

    // INDICES + TEXTOS
    std::vector<EntradaIndiceBinario> tablaIndices;
    std::vector<char> poolDeTextos;

    u32 offsetActual = 0;

    for (const auto& t : traduccionesOrdenadas) {
        tablaIndices.push_back({t.hashName, offsetActual});

        size_t len = t.text.size() + 1;
        poolDeTextos.insert(poolDeTextos.end(), t.text.begin(), t.text.end());
        poolDeTextos.push_back('\0');

        offsetActual += static_cast<u32>(len);
    }

    archivo.write(reinterpret_cast<const char*>(tablaIndices.data()),
                  tablaIndices.size() * sizeof(EntradaIndiceBinario));

    archivo.write(poolDeTextos.data(), poolDeTextos.size());

    archivo.close();

    std::cout << "Exportado: " << rutaSalida << "\n";
}

void mostrarUso(const char* nombrePrograma) {
    std::cerr << "Uso: " << nombrePrograma << " -i <ARCHIVO JSON> [-o CARPETA_SALIDA]\n";
}

int main(int argc, char* argv[]) {

    std::string rutaJson;
    std::string carpetaSalida;

    // PARSEO ARGUMENTOS
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            rutaJson = argv[i + 1];
        }
        else if (std::strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            carpetaSalida = argv[i + 1];
        }
    }

    if (rutaJson.empty()) {
        mostrarUso(argv[0]);
        return 1;
    }

    // Si no hay -o, usar nombre del JSON
    if (carpetaSalida.empty()) {
        fs::path p(rutaJson);
        carpetaSalida = p.stem().string();
    }

    // Crear carpeta salida
    try {
        fs::create_directories(carpetaSalida);
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Error creando carpeta: " << e.what() << "\n";
        return 1;
    }

    // LEER JSON
    std::ifstream archivoJson(rutaJson);
    if (!archivoJson.is_open()) {
        std::cerr << "No se pudo abrir JSON: " << rutaJson << "\n";
        return 1;
    }

    json datosJson;
    archivoJson >> datosJson;

    if (!datosJson.contains("langs") || !datosJson["langs"].is_array()) {
        std::cerr << "JSON inválido (faltan langs)\n";
        return 1;
    }

    // PROCESAR IDIOMAS
    for (const auto& langNode : datosJson["langs"]) {

        std::string nombreBase;

        if (langNode.contains("name") && langNode["name"].is_string()) {
            nombreBase = langNode["name"];
        }
        else if (langNode.contains("locale") && langNode["locale"].is_string()) {
            nombreBase = langNode["locale"];
        }
        else {
            continue;
        }

        std::string localeStr = langNode.value("locale", "unknown");

        Archivo miIdioma;
        miIdioma.hashLocale = calcular_hash(localeStr.c_str());

        if (langNode.contains("translations") && langNode["translations"].is_array()) {
            for (const auto& t : langNode["translations"]) {
                if (t.contains("id") && t.contains("text")) {
                    translation tr;
                    std::string idStr = t["id"];
                    tr.hashName = calcular_hash(idStr.c_str());
                    tr.text = t["text"];
                    miIdioma.translations.push_back(tr);
                }
            }
        }

        fs::path salida = fs::path(carpetaSalida) / (nombreBase + ".lang");

        exportarArchivoLang(miIdioma, localeStr, salida.string());
    }

    return 0;
}