#if defined(PLATFORM_3DS)
#pragma once

#include <3ds.h>
#include <citro3d.h>

typedef struct P200S_SHADER
{
    shaderProgram_s program;
    DVLB_s *shader;
    C3D_AttrInfo attrInfo;
	C3D_BufInfo bufInfo;
	C3D_ProcTex ptBlend;
    int uLoc_mvpMtx;
    C3D_Mtx mvpMtx;
    C3D_Mtx s_projTop, s_projBot;
} P200S_SHADER;

#ifdef __cplusplus
extern "C"
{
#endif

P200S_SHADER *P200S_Create();

bool P200S_CompileShader(P200S_SHADER *shader, void *data, u32 size);
bool P200S_LinkProgram(P200S_SHADER *shaderStruct);

bool P200S_Use(P200S_SHADER *shader);

bool P200S_Destroy(P200S_SHADER *shader);

#ifdef __cplusplus
}
#endif

#endif