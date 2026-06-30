#include "json.h"

#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>

#if defined(PLATFORM_PC)
#include "../nlohmann/json.hpp"
#include <pak_loader/pak_loader.h>

using json = nlohmann::json;

void CallText(const char *text)
{
    std::cout << text << std::endl;
}

static char *copyString(const char *str)
{
    if (!str)
        return nullptr;

    size_t len = strlen(str);

    char *out = (char *)malloc(len + 1);
    if (!out)
        return nullptr;

    memcpy(out, str, len + 1);
    return out;
}

void addImage(Info *info, image *img)
{
    if (!info || !img)
        return;

    image **tmp = (image **)realloc(
        info->images,
        (info->imageCount + 1) * sizeof(image *));

    if (!tmp)
        return; // NO pierdas el array original

    info->images = tmp;
    info->images[info->imageCount] = img;
    info->imageCount++;
}

Info getJsonInfo(const char *path)
{
    Info infoB{};
    infoB.w = 0;
    infoB.h = 0;
    infoB.tiles = 0;
    infoB.tilesValues = nullptr;
    infoB.images = nullptr;
    infoB.imageCount = 0;

    Info info = infoB;

    if (!path)
        return info;

    PAK_FILE *file = PAKL_LoadFile(path);
    if (!file)
        return info;

    // Se usa un try-catch que envuelve TODO el procesamiento para garantizar
    // que si el JSON está corrupto, la memoria parcial se libere correctamente.
    try
    {
        // Leer archivo completo a memoria
        PAKL_fseek(file, 0, SEEK_END);
        long size = PAKL_ftell(file);
        PAKL_rewind(file);

        if (size <= 0)
        {
            PAKL_CloseFile(file);
            return info;
        }

        std::string buffer;
        buffer.resize(size);

        PAKL_fread(buffer.data(), 1, size, file);
        PAKL_CloseFile(file); // Cerramos el archivo tan pronto como dejamos de usarlo
        file = nullptr;

        // Parse JSON desde string (puede lanzar excepciones si el JSON está mal formateado)
        json j = json::parse(buffer);

        info.w = j.value("width", 0);
        info.h = j.value("height", 0);
        //info.tileX = j.value("tilesX", 0);
        //info.tileY = j.value("tilesY", 0);

        if (j.contains("images") && j["images"].is_array())
        {
            std::string baseDir(path);
            size_t lastSlash = baseDir.find_last_of("/\\");

            if (lastSlash != std::string::npos) {
                baseDir = baseDir.substr(0, lastSlash + 1);
            } else {
                baseDir = "";
            }

            for (const auto &imgJson : j["images"])
            {
                image *img = (image *)malloc(sizeof(image));
                if (!img)
                    continue;

                img->x = imgJson.value("x", 0);
                img->y = imgJson.value("y", 0);

                std::string pathStr = baseDir + imgJson.value("path", "");
                img->path = copyString(pathStr.c_str());

                if (!img->path)
                {
                    free(img);
                    continue;
                }

                std::cout << "Adding image" << std::endl;
                addImage(&info, img);
                std::cout << "Added image" << std::endl;
            }
        }

        std::vector<Tile> totalTiles = {};

        if(j.contains("tiles") && j["tiles"].is_array())
        {
            for(const auto &tile : j["tiles"])
            {
                if(tile.is_object() && tile.contains("top") && tile["top"].is_number_float() && tile.contains("left") && tile["left"].is_number_float() && tile.contains("right") && tile["right"].is_number_float() && tile.contains("bottom") && tile["bottom"].is_number_float())
                {
                    Tile t;
                    t.top = tile["top"];
                    t.left = tile["left"];
                    t.right = tile["right"];
                    t.bottom = tile["bottom"];
                    totalTiles.push_back(t);
                    infoB.tiles++;
                }
            }
            infoB.tilesValues = (Tile*)malloc(sizeof(Tile)*totalTiles.size());
            *infoB.tilesValues = *totalTiles.data();
            totalTiles.clear();
        }

        return info;
    }
    catch (...)
    {
        std::cout << "[Error] El JSON está corrupto o falló la lectura. Limpiando..." << std::endl;
        if (file) {
            PAKL_CloseFile(file);
        }
        // Si ya habíamos añadido imágenes antes del crash/excepción, las liberamos
        freeInfo(&info); 
        return infoB;
    }
}
#endif

void freeInfo(Info *info)
{
    if (!info)
        return;

    std::cout << "Free info" << std::endl;
    if (info->images)
    {
        for (size_t i = 0; i < info->imageCount; i++)
        {
            image *img = info->images[i];

            if (!img)
                continue;

            std::cout << "Image to free index: " << i << std::endl;
            if (img->path)
            {
                free(img->path);
                img->path = nullptr;
                std::cout << "Free path" << std::endl;
            }

            // Nota: 'free()' en C puro jamás lanza excepciones de C++. 
            // Si crasheaba aquí, era porque el puntero 'img' ya era inválido o corrupto.
            free(img);
            info->images[i] = nullptr;
            std::cout << "Free image" << std::endl;
        }

        free(info->images);
        info->images = nullptr;
        std::cout << "Free Array" << std::endl;
    }

    info->imageCount = 0;
}
