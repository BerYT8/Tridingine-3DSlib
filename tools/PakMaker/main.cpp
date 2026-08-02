#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include <cstring>
#include <algorithm>
#include <cstdint> // Necesario para uint32_t y uint16_t
#include <regex>   // Reemplazamos fnmatch por expresiones regulares nativas

namespace fs = std::filesystem;

// 1. Estructuras de datos del formato PAKv1.0.0
#pragma pack(push, 1) 
struct PAK_Index_Entry {
    char path[256];        // 256 bytes: Ruta relativa
    uint32_t offset;       // 4 bytes: Desplazamiento RELATIVO (Soporta hasta 4GB globales)
    uint32_t size;         // 4 bytes: Tamaño del archivo en bytes (Soporta hasta 4GB por archivo)
};
#pragma pack(pop)

struct PendingFile {
    fs::path absolute_path;
    std::string relative_path_str;
    uintmax_t size;
};

// Convierte un patrón de usuario (ej: "examples/*/examples") en una Expresión Regular válida
std::regex ConvertirPatronARegex(std::string patron) {
    std::string regex_str = "";
    
    // Escapar caracteres especiales de regex pero transformar los '*'
    for (char c : patron) {
        if (c == '*') {
            regex_str += ".*"; // En regex, .* significa "cualquier cosa, incluyendo barras /"
        } else if (c == '.' || c == '\\' || c == '+' || c == '?' || c == '^' || c == '$' || c == '[' || c == ']' || c == '{' || c == '}' || c == '(' || c == ')' || c == '|') {
            regex_str += "\\";
            regex_str += c;
        } else {
            regex_str += c;
        }
    }
    
    // Permite que coincida si el patrón es parte de la ruta, o si el archivo está dentro de esa ruta
    // Usamos std::regex::icase para que no importe si es "Orbit" u "orbit"
    return std::regex(".*" + regex_str + ".*", std::regex::ECMAScript | std::regex::icase);
}

// Función auxiliar para comprobar si un archivo debe ser excluido
bool DeberiaExcluirse(const fs::path& ruta_relativa, const std::vector<std::regex>& regex_exclusiones, const std::vector<std::string>& exclusiones_simples) {
    std::string ruta_completa_str = ruta_relativa.generic_string();

    // 1. Comprobación mediante las Regex generadas por los comodines
    for (const auto& re : regex_exclusiones) {
        if (std::regex_match(ruta_completa_str, re)) {
            return true;
        }
    }

    // 2. Verificación componente por componente para nombres simples (ej. "build" o "archivo.txt")
    for (const auto& componente : ruta_relativa) {
        std::string comp_str = componente.generic_string();
        for (const auto& excl : exclusiones_simples) {
            // Comparación insensible a mayúsculas/minúsculas para nombres de carpetas
            if (std::equal(comp_str.begin(), comp_str.end(), excl.begin(), excl.end(),
                           [](char a, char b) { return std::tolower(a) == std::tolower(b); })) {
                return true;
            }
        }
    }
    return false;
}

// 2. Lógica principal de empaquetado
bool CrearPaquete(const fs::path& carpeta_origen, const std::string& archivo_salida, const std::vector<std::string>& exclusiones) {
    if (!fs::exists(carpeta_origen) || !fs::is_directory(carpeta_origen)) {
        std::cerr << "Error: La carpeta especificada no existe.\n";
        return false;
    }

    // Precompilar los patrones con comodines a expresiones regulares para mayor rendimiento
    std::vector<std::regex> regex_exclusiones;
    std::vector<std::string> exclusiones_simples;
    for (const auto& excl : exclusiones) {
        if (excl.find('*') != std::string::npos) {
            regex_exclusiones.push_back(ConvertirPatronARegex(excl));
        } else {
            exclusiones_simples.push_back(excl);
        }
    }

    std::vector<PendingFile> lista_archivos;

    for (const auto& entrada : fs::recursive_directory_iterator(carpeta_origen)) {
        if (fs::is_regular_file(entrada.path())) {
            fs::path relativa = fs::relative(entrada.path(), carpeta_origen);
            
            // FILTRO DE EXCLUSIÓN CRÍTICO
            if (DeberiaExcluirse(relativa, regex_exclusiones, exclusiones_simples)) {
                //std::cout << "Excluido: " << relativa.generic_string() << "\n";
                continue; 
            }

            PendingFile pf;
            pf.absolute_path = entrada.path();
            pf.relative_path_str = relativa.generic_string();
            pf.size = fs::file_size(entrada.path());

            if (pf.relative_path_str.length() >= 256) {
                std::cerr << "Error: La ruta " << pf.relative_path_str << " supera el límite de 255 caracteres.\n";
                return false;
            }

            if (pf.size > 4294967295ULL) {
                std::cerr << "Error: El archivo " << pf.relative_path_str << " supera los 4GB individuales.\n";
                return false;
            }

            lista_archivos.push_back(pf);
        }
    }

    if (lista_archivos.empty()) {
        std::cerr << "Error: No se encontraron archivos para empaquetar.\n";
        return false;
    }

    std::ofstream pak(archivo_salida, std::ios::binary);
    if (!pak.is_open()) {
        std::cerr << "Error: No se pudo crear el archivo " << archivo_salida << "\n";
        return false;
    }

    const char* cabecera = "PAKv1.0.0";
    pak.write(cabecera, 9);

    uint32_t bytes_lista_indices = static_cast<uint32_t>(lista_archivos.size() * sizeof(PAK_Index_Entry));
    pak.write(reinterpret_cast<char*>(&bytes_lista_indices), sizeof(bytes_lista_indices));

    uint32_t offset_relativo_actual = 0;
    std::vector<PAK_Index_Entry> tabla_indices;

    for (auto& pf : lista_archivos) {
        PAK_Index_Entry entrada;
        
        std::memset(entrada.path, 0, sizeof(entrada.path));
        std::strncpy(entrada.path, pf.relative_path_str.c_str(), sizeof(entrada.path) - 1);
        
        entrada.offset = offset_relativo_actual;
        entrada.size = static_cast<uint32_t>(pf.size);
        
        tabla_indices.push_back(entrada);

        if (static_cast<unsigned long long>(offset_relativo_actual) + pf.size > 4294967295ULL) {
            std::cerr << "Error: El tamaño total de los archivos empaquetados supera los 4GB admitidos por el formato.\n";
            pak.close();
            fs::remove(archivo_salida);
            return false;
        }

        offset_relativo_actual += static_cast<uint32_t>(pf.size);
    }

    pak.write(reinterpret_cast<char*>(tabla_indices.data()), bytes_lista_indices);

    constexpr size_t BUFFER_SIZE = 1024 * 1024;
    std::vector<char> buffer(BUFFER_SIZE);

    for (const auto& pf : lista_archivos)
    {
        std::ifstream src(pf.absolute_path, std::ios::binary);
        if (!src) {
            std::cerr << "Error crítico: No se pudo abrir " << pf.absolute_path << "\n";
            pak.close();
            return false;
        }

        while (src) {
            src.read(buffer.data(), BUFFER_SIZE);
            std::streamsize bytes = src.gcount();
            if (bytes > 0) pak.write(buffer.data(), bytes);
        }
        src.close();

        std::cout << "Empaquetado: " << pf.relative_path_str << " [" << pf.size << " bytes]\n";
    }
    pak.flush();

    std::cout << "Tamano esperado: " << (9 + sizeof(uint32_t) + bytes_lista_indices + offset_relativo_actual) << "\n";
    std::cout << "Tamano real: " << pak.tellp() << "\n";

    pak.close();
    std::cout << "\n¡Paquete creado con éxito!: " << archivo_salida << " (" << lista_archivos.size() << " archivos)\n";
    return true;
}

void mostrar_uso(const char* nombre) {
    std::cout << "Uso:\n";
    std::cout << "  "  << nombre << " -c \"[ruta_de_la_carpeta]\" [-o \"[archivo_salida.pak]\"] [-e \"exclusión1\" \"exclusión2\" ...]\n";
}

int main(int argc, char* argv[]) {
    std::string carpeta_objetivo = "";
    std::string archivo_salida = "datos.pak";
    std::vector<std::string> exclusiones;

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--help") == 0) {
            mostrar_uso(argv[0]); // Corregido argv por argv[0]
            return 1;
        }
        if (std::strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            carpeta_objetivo = argv[i + 1];
            i++; 
        }
        else if (std::strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            archivo_salida = argv[i + 1];
            i++; 
        }
        else if (std::strcmp(argv[i], "-e") == 0) {
            // Se detiene si el siguiente argumento es el final, o si empieza por '-' (parámetro nuevo)
            while (i + 1 < argc && argv[i + 1][0] != '-') {
                exclusiones.push_back(argv[i + 1]);
                i++;
            }
        }
    }

    if (carpeta_objetivo.empty()) {
        mostrar_uso(argv[0]); // Corregido argv por argv[0]
        return 1;
    }

    if (CrearPaquete(carpeta_objetivo, archivo_salida, exclusiones)) {
        return 0;
    }

    return 1;
}
