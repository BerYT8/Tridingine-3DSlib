#pragma once

#include "material.h"
#include "quaternion.h"

#include <stdbool.h>

typedef struct Model3D Model3D;

typedef struct Camera3D
{
    Vec3 pos;
    Vec3 rot;
    float fov;
} Camera3D;

#ifdef __cplusplus
extern "C"
{
#endif

void D3D_AddFront(Camera3D *c, float amount);
void D3D_AddRight(Camera3D *c, float amount);
void D3D_AddUp(Camera3D *c, float amount);

Vec3 D3D_LookAt(Vec3 from, Vec3 to);
Vec3 D3D_ForwardToRotation(Vec3 forward);
Vec3 D3D_RotationToForward(const Vec3& rot);
void D3D_AddForwardPos(Camera3D *c, Vec3 v);

Vec3 D3D_Camera_GetForward(Camera3D *c);

Vec3 D3D_Camera_GetRight(Camera3D *c);

Vec3 D3D_Camera_GetUp(Camera3D *c);

void D3D_SetCameraPos(Camera3D *c, Vec3 pos);
void D3D_SetCameraRot(Camera3D *c, Vec3 rot);
void D3D_SetCameraFov(Camera3D *c, float fov);

Vec3 D3D_GetCameraPos(Camera3D *c);
Vec3 D3D_GetCameraRot(Camera3D *c);
float D3D_GetCameraFov(Camera3D *c);

bool D3D_Init();

void D3D_Prepare();

Model3D *D3D_LoadModel3D(const char* path);

bool D3D_DrawModel(Model3D *model, Vec3 position, Vec3 rotation, Vec3 scale, Material material, Camera3D *camera);

bool D3D_DrawCuboid(Vec3 position, Vec3 rotation, Vec3 scale, Vec3 center, Material material, Camera3D *camera);
bool D3D_DrawSphere(Vec3 position, Vec3 rotation, Vec3 scale, Vec3 center, Material material, Camera3D *camera);

void D3D_Exit();

#ifdef __cplusplus
}
#endif