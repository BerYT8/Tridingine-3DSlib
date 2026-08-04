#include <pak_loader/pak_loader.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <regex>
#include <vector>
#include <algorithm>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#endif

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
        case '*': r += ".*"; break;
        case '?': r += "."; break;

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
        }
    }

    r += "$";
    return r;
}

bool matchPattern(const std::string& text, const std::string& pattern)
{
    return std::regex_match(
        text,
        std::regex(
            wildcardToRegex(pattern),
            std::regex_constants::icase));
}

bool matchesAny(const std::string& text,
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
            continue;
        }

        if(strcmp(argv[i], "--no-examples") == 0)
        {
            excludePatterns.emplace_back("examples/*");
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
            while (i + 1 < argc && argv[i + 1][0] != '-')
                onlyPatterns.emplace_back(argv[++i]);

            continue;
        }

        if (strcmp(argv[i], "--exclude") == 0)
        {
            while (i + 1 < argc && argv[i + 1][0] != '-')
                excludePatterns.emplace_back(argv[++i]);

            continue;
        }
    }

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

    const void* ptr = nullptr;
    size_t size = 0;
    std::vector<char> bufferPakExterno;

#if defined(_WIN32)
    HRSRC res = FindResource(nullptr, TEXT("PAKFILE"), RT_RCDATA);
    if (res)
    {
        HGLOBAL data = LoadResource(nullptr, res);
        ptr = LockResource(data);
        size = SizeofResource(nullptr, res);
    }
#endif

    // Si no estamos en Windows o falló el recurso integrado, buscamos el archivo físico en Linux
    if (!ptr)
    {
        fs::path rutaPrograma = fs::absolute(argv[0]).parent_path();
        fs::path rutaPak = rutaPrograma / "template.pak";

        if (!fs::exists(rutaPak))
        {
            // Intento secundario en el directorio de trabajo actual
            rutaPak = "template.pak";
        }

        std::ifstream archivoPak(rutaPak, std::ios::binary | std::ios::ate);
        if (!archivoPak.is_open())
        {
            std::cerr << "ERROR: No se encontró el recurso integrado ni el archivo externo 'template.pak'\n";
            return 1;
        }

        size = archivoPak.tellg();
        bufferPakExterno.resize(size);
        archivoPak.seekg(0, std::ios::beg);
        archivoPak.read(bufferPakExterno.data(), size);
        archivoPak.close();

        ptr = bufferPakExterno.data();
    }

    PAKL_SetPakFromMem(ptr, size);

    size_t count = PAKL_GetFileCount();

    for (size_t i = 0; i < count; i++)
    {
        const char* filename = PAKL_GetFileName(i);

        std::string file = filename;

        // Si existe --only, el archivo debe coincidir con alguno.
        if (!onlyPatterns.empty() &&
            !matchesAny(file, onlyPatterns))
        {
            continue;
        }

        // Siempre se aplica --exclude.
        if (matchesAny(file, excludePatterns))
        {
            continue;
        }

        fs::path outFile = output / filename;

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

        PAK_FILE* fileHandle = PAKL_LoadFile(filename);

        if (!fileHandle)
            continue;

        size_t filesize = PAKL_GetFileSize(fileHandle);

        fs::create_directories(outFile.parent_path());

        std::ofstream out(outFile, std::ios::binary);

        constexpr size_t BUFFER = 64 * 1024;
        u8 buffer[BUFFER];

        size_t remaining = filesize;

        while (remaining)
        {
            size_t request = std::min(remaining, BUFFER);

            size_t readed = PAKL_fread(buffer, 1, request, fileHandle);

            if (readed == 0)
                break;

            out.write(reinterpret_cast<char*>(buffer), readed);

            remaining -= readed;
        }

        out.close();
        PAKL_CloseFile(fileHandle);
    }

    PAKL_ClosePak();

    return 0;
}
