#if defined(PLATFORM_PC)

#include <pak_loader/pak_loader.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>
#define NOMINMAX
#include <windows.h>
#include <string>
#include <ints_defs.h>

namespace fs = std::filesystem;


int main(int argc, char* argv[])
{
    fs::path output;

    bool updateMode = false;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--update") == 0)
        {
            updateMode = true;
            continue;
        }
        if (strcmp(argv[i], "-o") == 0)
        {
            output = argv[i+1];
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

    HRSRC res = FindResource(
        nullptr,
        TEXT("PAKFILE"),
        RT_RCDATA
    );

    if (!res)
    {
        std::cout << "ERROR: recurso PAKFILE no encontrado\n";
        return 1;
    }

    DWORD resourceSize = SizeofResource(nullptr, res);

    HGLOBAL data = LoadResource(nullptr, res);

    const void* ptr = LockResource(data);
    size_t size = SizeofResource(nullptr, res);

    PAKL_SetPakFromMem(ptr, size);

    size_t count = PAKL_GetFileCount();

    for (size_t i = 0; i < count; i++)
    {
        const char* filename = PAKL_GetFileName(i);

        fs::path outFile = output / filename;

        if (updateMode && fs::exists(outFile))
        {
            fs::path p(filename);
            std::string name = p.filename().string();

            if (name == "CMakeLists.txt" ||
                name == "game.json" ||
                name == "main.cpp")
            {
                continue;
            }
        }

        PAK_FILE* file = PAKL_LoadFile(filename);

        if (!file)
            continue;

        size_t filesize = PAKL_GetFileSize(file);

        fs::create_directories(outFile.parent_path());

        std::ofstream out(outFile, std::ios::binary);

        constexpr size_t BUFFER = 64 * 1024;
        u8 buffer[BUFFER];

        size_t remaining = filesize;

        while (remaining)
        {
            size_t request = std::min(remaining, BUFFER);

            size_t readed = PAKL_fread(buffer, 1, request, file);

            if (readed == 0)
                break;

            out.write(reinterpret_cast<char*>(buffer), readed);

            remaining -= readed;
        }

        out.close();
        PAKL_CloseFile(file);
    }

    PAKL_ClosePak();

    return 0;
}

#endif