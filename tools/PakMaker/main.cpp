#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include <cstring>
#include <algorithm>
#include <cstdint> // Necesario para uint32_t y uint16_t

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

// 2. Lógica principal de empaquetado
bool CrearPaquete(const fs::path& carpeta_origen, const std::string& archivo_salida) {
    if (!fs::exists(carpeta_origen) || !fs::is_directory(carpeta_origen)) {
        std::cerr << "Error: La carpeta especificada no existe.\n";
        return false;
    }

    std::vector<PendingFile> lista_archivos;

    for (const auto& entrada : fs::recursive_directory_iterator(carpeta_origen)) {
        if (fs::is_regular_file(entrada.path())) {
            PendingFile pf;
            pf.absolute_path = entrada.path();
            fs::path relativa = fs::relative(entrada.path(), carpeta_origen);
            pf.relative_path_str = relativa.generic_string();
            pf.size = fs::file_size(entrada.path());

            if (pf.relative_path_str.length() >= 256) {
                std::cerr << "Error: La ruta " << pf.relative_path_str << " supera el límite de 255 caracteres.\n";
                return false;
            }

            // NUEVO LÍMITE: Validar que un archivo individual no supere los 4GB (Límite de uint32_t)
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

    // A. Escribir Cabecera (8 bytes)
    const char* cabecera = "PAKv1.0.0";
    pak.write(cabecera, 9);

    // B. Calcular y escribir el tamaño en bytes de la lista de índices (Cambiado a uint32_t por seguridad)
    uint32_t bytes_lista_indices = static_cast<uint32_t>(lista_archivos.size() * sizeof(PAK_Index_Entry));
    pak.write(reinterpret_cast<char*>(&bytes_lista_indices), sizeof(bytes_lista_indices));

    // C. Calcular los offsets relativos de cada archivo
    uint32_t offset_relativo_actual = 0;
    std::vector<PAK_Index_Entry> tabla_indices;

    for (auto& pf : lista_archivos) {
        PAK_Index_Entry entrada;
        
        std::memset(entrada.path, 0, sizeof(entrada.path));
        std::strncpy(entrada.path, pf.relative_path_str.c_str(), sizeof(entrada.path) - 1);
        
        entrada.offset = offset_relativo_actual;
        entrada.size = static_cast<uint32_t>(pf.size);
        
        tabla_indices.push_back(entrada);

        // NUEVO LÍMITE: Control de desbordamiento global del paquete a 4GB
        if (static_cast<unsigned long long>(offset_relativo_actual) + pf.size > 4294967295ULL) {
            std::cerr << "Error: El tamaño total de los archivos empaquetados supera los 4GB admitidos por el formato.\n";
            pak.close();
            fs::remove(archivo_salida);
            return false;
        }

        offset_relativo_actual += static_cast<uint32_t>(pf.size);
    }

    // D. Escribir la tabla de índices en el archivo binario
    pak.write(reinterpret_cast<char*>(tabla_indices.data()), bytes_lista_indices);

    // E. Escribir el contenido real de los archivos uno por uno
    // E. Escribir el contenido real de los archivos uno por uno
    constexpr size_t BUFFER_SIZE = 1024 * 1024;
    std::vector<char> buffer(BUFFER_SIZE);

    for (const auto& pf : lista_archivos)
    {
        std::ifstream src(pf.absolute_path, std::ios::binary);

        if (!src)
        {
            std::cerr << "Error crítico: No se pudo abrir "
                    << pf.absolute_path << "\n";
            pak.close();
            return false;
        }

        while (src)
        {
            src.read(buffer.data(), BUFFER_SIZE);
            std::streamsize bytes = src.gcount();

            if (bytes > 0)
                pak.write(buffer.data(), bytes);
        }

        src.close();

        std::cout << "Empaquetado: "
                << pf.relative_path_str
                << " ["
                << pf.size
                << " bytes]\n";
    }
    pak.flush();

    std::cout
        << "Tamano esperado: "
        << (9 + sizeof(uint32_t) +
            bytes_lista_indices +
            offset_relativo_actual)
        << "\n";

    std::cout
        << "Tamano real: "
        << pak.tellp()
        << "\n";

    pak.close();
    std::cout << "\n¡Paquete creado con éxito!: " << archivo_salida << " (" << lista_archivos.size() << " archivos)\n";
    return true;
}

void mostrar_uso(const char* nombre)
{
    std::cout << "Uso:\n";
    std::cout << "  "  << nombre << " -c \"[ruta_de_la_carpeta]\" [-o \"[archivo_salida.pak]\"]\n";
}

// 3. Punto de entrada (Manejo de argumentos)
int main(int argc, char* argv[]) {
    std::string carpeta_objetivo = "";
    std::string archivo_salida = "datos.pak";

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--help") == 0 && i + 1 < argc) {
            mostrar_uso(argv[0]);
            return 1;
        }
        if (std::strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            carpeta_objetivo = argv[i + 1];
        }
        if (std::strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            archivo_salida = argv[i + 1];
        }
    }

    if (carpeta_objetivo.empty()) {
        mostrar_uso(argv[0]);
        return 1;
    }

    if (CrearPaquete(carpeta_objetivo, archivo_salida)) {
        return 0;
    }

    return 1;
}