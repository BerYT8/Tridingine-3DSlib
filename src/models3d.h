#pragma once
#include <maths.h>
#include <vector>
#include <ints_defs.h>

typedef Vec3 VertexPos;
typedef Vec2 UV;

typedef struct Vertex
{
    u16 vertex;
    UV uv;
} Vertex;

Vertex MakeVertex(u16 pos, UV uv)
{
    Vertex v;
    v.vertex = pos;
    v.uv = uv;
    return v;
}
#ifdef __cplusplus
typedef struct Face
{
    Vertex vertex[3];
} Face;

typedef struct Model3D
{
    std::vector<VertexPos> vectorPos;
    std::vector<Face> faces;
} Model3D;

std::vector<Model3D*> models = {};
#else
typedef struct Face Face;
typedef struct Model3D Model3D;
#endif