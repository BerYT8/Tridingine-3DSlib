#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include <sstream>
#include <cstdint>
#include <algorithm>

#include <models3d.h>


struct OBJIndex
{
    u16 vertex;
    u16 uv;
};

static OBJIndex parseOBJIndex(const std::string& token)
{
    OBJIndex idx;
    idx.vertex = 0;
    idx.uv = 0;

    size_t s1 = token.find('/');

    if (s1 == std::string::npos)
    {
        idx.vertex = (u16)(std::stoi(token) - 1);
        return idx;
    }

    idx.vertex = (u16)(std::stoi(token.substr(0, s1)) - 1);

    size_t s2 = token.find('/', s1 + 1);

    if (s2 == std::string::npos)
    {
        if (s1 + 1 < token.size())
            idx.uv = (u16)(std::stoi(token.substr(s1 + 1)) - 1);
    }
    else
    {
        if (s2 > s1 + 1)
            idx.uv = (u16)(std::stoi(token.substr(s1 + 1, s2 - s1 - 1)) - 1);
    }

    return idx;
}

// ---------------- OBJ PARSER ----------------

static std::vector<std::string> split(const std::string& s)
{
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> result;
    while (ss >> item) result.push_back(item);
    return result;
}

static u16 parseIndex(const std::string& token)
{
    // formato: v/vt/vn o v//vn o v
    size_t pos = token.find('/');
    if (pos == std::string::npos)
        return (u16)(std::stoi(token) - 1);

    return (u16)(std::stoi(token.substr(0, pos)) - 1);
}


// ---------------- LOAD OBJ ----------------

bool loadOBJ(const std::string& path, Model3D& model)
{
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;

    std::vector<Vec2> objUVs;

    while (std::getline(file, line))
    {
        if (line.rfind("v ", 0) == 0)
        {
            Vec3 v;
            sscanf(line.c_str(), "v %f %f %f", &v.x, &v.y, &v.z);
            model.vectorPos.push_back(v);
        }
        else if (line.rfind("f ", 0) == 0)
        {
            auto tokens = split(line.substr(2));

            std::vector<OBJIndex> indices;

            for (auto& t : tokens)
                indices.push_back(parseOBJIndex(t));

            // TRIANGULACIÓN (fan)
            for (size_t i = 1; i + 1 < indices.size(); i++)
            {
                Face f;

                f.vertex[0].vertex = indices[0].vertex;
                f.vertex[1].vertex = indices[i].vertex;
                f.vertex[2].vertex = indices[i + 1].vertex;

                f.vertex[0].uv = objUVs[indices[0].uv];
                f.vertex[1].uv = objUVs[indices[i].uv];
                f.vertex[2].uv = objUVs[indices[i + 1].uv];

                model.faces.push_back(f);
            }
        }
        else if (line.rfind("vt ", 0) == 0)
        {
            Vec2 uv;
            sscanf(line.c_str(), "vt %f %f", &uv.x, &uv.y);

            // Muchos motores usan origen arriba.
            uv.y = 1.0f - uv.y;

            objUVs.push_back(uv);
        }
    }

    return true;
}


// ---------------- EXPORT M3DS ----------------

bool saveM3DS(const std::string& path, const Model3D& model)
{
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;

    // HEADER
    char header[4] = { 'M','3','D','S' };
    out.write(header, 4);

    u32 vcount = (u32)model.vectorPos.size();
    u32 fcount = (u32)model.faces.size();

    out.write((char*)&vcount, sizeof(u32));
    out.write((char*)&fcount, sizeof(u32));

    // ---------------- VERTICES ----------------
    for (const auto& v : model.vectorPos)
    {
        out.write((char*)&v.x, sizeof(float));
        out.write((char*)&v.y, sizeof(float));
        out.write((char*)&v.z, sizeof(float));
    }

    // ---------------- FACES ----------------
    for (const auto& f : model.faces)
    {
        for (int i = 0; i < 3; i++)
        {
            u16 idx = f.vertex[i].vertex;
            out.write((char*)&idx, sizeof(u16));

            out.write((char*)&f.vertex[i].uv.x, sizeof(float));
            out.write((char*)&f.vertex[i].uv.y, sizeof(float));
        }
    }

    return true;
}


// ---------------- MAIN ----------------

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        std::cout << "Uso: " << argv[0] << " modelo.obj -o salida.m3ds\n";
        return 1;
    }

    std::string input = argv[1];
    std::string output = argv[3];

    Model3D model;

    if (!loadOBJ(input, model))
    {
        std::cout << "Error cargando OBJ\n";
        return 1;
    }

    if (!saveM3DS(output, model))
    {
        std::cout << "Error guardando M3DS\n";
        return 1;
    }

    std::cout << "Conversión completada correctamente\n";
    return 0;
}