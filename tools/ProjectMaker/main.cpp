#include "template_pak.h"
#include <pak_loader/pak_loader.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <regex>
#include <vector>
#include <algorithm>
#include <string>

#include <ints_defs.h>

namespace fs = std::filesystem;

void mostrarUso(const char* nombrePrograma)
{
    std::cerr << "Uso:\n";
    std::cerr << "  " << nombrePrograma << " --update -o <CARPETA>\n";
    std::cerr << "  " << nombrePrograma << " -o <CARPETA>\n";
    std::cerr << "  " << nombrePrograma << " [--only <patron> ...]\n";
    std::cerr << "  " << nombrePrograma << " [--exclude <patron> ...]\n";
    std::cerr << "  " << nombrePrograma << " [--no-examples (optional)]\n";
}

std::string wildcardToRegex(const std::string& pattern)
{
    std::string r = "^";

    for (char c : pattern)
    {
        switch (c)
        {
        case '*':
            r += ".*";
            break;

        case '?':
            r += ".";
            break;

        case '.':
            r += "\\.";
            break;

        case '\\':
        case '/':
            r += "[\\\\/]";
            break;

        case '+':
        case '(':
        case ')':
        case '^':
        case '$':
        case '|':
        case '{':
        case '}':
        case '[':
        case ']':
            r += '\\';
            r += c;
            break;

        default:
            r += c;
            break;
        }
    }

    r += "$";

    return r;
}

bool matchPattern(
    const std::string& text,
    const std::string& pattern)
{
    return std::regex_match(
        text,
        std::regex(
            wildcardToRegex(pattern),
            std::regex_constants::icase
        )
    );
}

bool matchesAny(
    const std::string& text,
    const std::vector<std::string>& patterns)
{
    for (const auto& p : patterns)
    {
        if (matchPattern(text, p))
            return true;
    }

    return false;
}

int main(int argc, char* argv[])
{
    fs::path output;

    bool updateMode = false;

    std::vector<std::string> onlyPatterns;
    std::vector<std::string> excludePatterns;

    // ============================================================
    // PROCESAR ARGUMENTOS
    // ============================================================

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--help") == 0)
        {
            mostrarUso(argv[0]);
            return 1;
        }

        if (strcmp(argv[i], "--update") == 0)
        {
            updateMode = true;
            excludePatterns.emplace_back("examples/*");
            continue;
        }

        if (strcmp(argv[i], "--no-examples") == 0)
        {
            excludePatterns.emplace_back("examples/*");
            continue;
        }

        if (strcmp(argv[i], "-o") == 0)
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Falta la carpeta tras -o\n";
                return 1;
            }

            output = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "--only") == 0)
        {
            while (i + 1 < argc &&
                   argv[i + 1][0] != '-')
            {
                onlyPatterns.emplace_back(argv[++i]);
            }

            continue;
        }

        if (strcmp(argv[i], "--exclude") == 0)
        {
            while (i + 1 < argc &&
                   argv[i + 1][0] != '-')
            {
                excludePatterns.emplace_back(argv[++i]);
            }

            continue;
        }
    }

    // ============================================================
    // CARPETA DE SALIDA
    // ============================================================

    if (output.empty())
    {
        std::cout << "No se han pasado parametros.\n";
        std::cout << "Introduce la carpeta de salida: ";

        std::string input;
        std::getline(std::cin, input);

        output = input;

        if (output.empty())
        {
            std::cout << "Carpeta invalida\n";
            return 1;
        }
    }

    // ============================================================
    // PAK EMBEBIDO
    //
    // template_pak.h es generado automáticamente por CMake
    // a partir de template.pak.
    //
    // No se necesita ningún archivo externo en runtime.
    // ============================================================

    PAKL_SetPakFromMem(
        template_pak,
        template_pak_size
    );

    // ============================================================
    // COMPROBAR QUE EL PAK SE HA CARGADO
    // ============================================================

    size_t count = PAKL_GetFileCount();

    // ============================================================
    // EXTRAER ARCHIVOS
    // ============================================================

    for (size_t i = 0; i < count; ++i)
    {
        const char* filename = PAKL_GetFileName(i);

        if (!filename)
            continue;

        std::string file = filename;

        // --------------------------------------------------------
        // --only
        //
        // Si existe --only, el archivo debe coincidir con alguno.
        // --------------------------------------------------------

        if (!onlyPatterns.empty() &&
            !matchesAny(file, onlyPatterns))
        {
            continue;
        }

        // --------------------------------------------------------
        // --exclude
        //
        // Siempre se aplica.
        // --------------------------------------------------------

        if (matchesAny(file, excludePatterns))
        {
            continue;
        }

        // --------------------------------------------------------
        // Ruta de salida
        // --------------------------------------------------------

        fs::path outFile = output / filename;

        // --------------------------------------------------------
        // --update
        //
        // Si el archivo ya existe, ciertos archivos no se
        // sobrescriben.
        // --------------------------------------------------------

        if (updateMode && fs::exists(outFile))
        {
            fs::path p(filename);

            std::string name = p.filename().string();

            if (name == "CMakeLists.txt" ||
                name == "game.json" ||
                name == "main.cpp" ||
                name == "icon.png" ||
                name == "banner_def.png" ||
                name == "banner_audio.wav")
            {
                continue;
            }
        }

        // --------------------------------------------------------
        // Abrir archivo dentro del PAK
        // --------------------------------------------------------

        PAK_FILE* fileHandle =
            PAKL_LoadFile(filename);

        if (!fileHandle)
        {
            std::cerr
                << "No se pudo abrir: "
                << filename
                << "\n";

            continue;
        }

        // --------------------------------------------------------
        // Tamaño
        // --------------------------------------------------------

        size_t filesize =
            PAKL_GetFileSize(fileHandle);

        // --------------------------------------------------------
        // Crear directorios
        // --------------------------------------------------------

        std::error_code ec;

        fs::create_directories(
            outFile.parent_path(),
            ec
        );

        if (ec)
        {
            std::cerr
                << "No se pudo crear el directorio para: "
                << outFile
                << "\n";

            PAKL_CloseFile(fileHandle);

            continue;
        }

        // --------------------------------------------------------
        // Crear archivo de salida
        // --------------------------------------------------------

        std::ofstream out(
            outFile,
            std::ios::binary
        );

        if (!out.is_open())
        {
            std::cerr
                << "No se pudo crear: "
                << outFile
                << "\n";

            PAKL_CloseFile(fileHandle);

            continue;
        }

        // --------------------------------------------------------
        // Copiar por bloques
        // --------------------------------------------------------

        constexpr size_t BUFFER = 64 * 1024;

        u8 buffer[BUFFER];

        size_t remaining = filesize;

        while (remaining > 0)
        {
            size_t request =
                std::min(
                    remaining,
                    BUFFER
                );

            size_t readed =
                PAKL_fread(
                    buffer,
                    1,
                    request,
                    fileHandle
                );

            if (readed == 0)
            {
                std::cerr
                    << "Error leyendo: "
                    << filename
                    << "\n";

                break;
            }

            out.write(
                reinterpret_cast<const char*>(buffer),
                static_cast<std::streamsize>(readed)
            );

            if (!out)
            {
                std::cerr
                    << "Error escribiendo: "
                    << outFile
                    << "\n";

                break;
            }

            remaining -= readed;
        }

        // --------------------------------------------------------
        // Cerrar archivo
        // --------------------------------------------------------

        out.close();

        PAKL_CloseFile(fileHandle);
    }

    // ============================================================
    // CERRAR PAK
    // ============================================================

    PAKL_ClosePak();

    return 0;
}