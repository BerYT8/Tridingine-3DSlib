#if defined(PLATFORM_3DS)
#include "pica.shader.h"

P200S_SHADER *P200S_Create()
{
    return new P200S_SHADER();
}

bool P200S_CompileShader(P200S_SHADER *shader, void *data, u32 size)
{
    if(!shader)
        return false;

    shader->shader = DVLB_ParseFile((u32*)data, size);

    if(!shader->shader)
        return false;

    shaderProgramInit(&shader->program);
	shaderProgramSetVsh(&shader->program, &shader->shader->DVLE[0]);

    return true;
}
bool P200S_LinkProgram(P200S_SHADER *shader)
{
    if(!shader)
        return false;

    C3D_BindProgram(&shader->program);
	C3D_SetAttrInfo(&shader->attrInfo);
	C3D_SetBufInfo(&shader->bufInfo);

    return true;
}

bool P200S_Use(P200S_SHADER *shader)
{
    if(!shader)
        return false;

    if(shader->uLoc_mvpMtx >= 0)
    {
        C3D_FVUnifMtx4x4(
            GPU_VERTEX_SHADER,
            shader->uLoc_mvpMtx,
            &shader->mvpMtx);
    }

    return true;
}

bool P200S_Destroy(P200S_SHADER *shader)
{
    if(!shader)
        return false;

    shaderProgramFree(&shader->program);
    DVLB_Free(shader->shader);

    delete shader;

    return true;
}

#endif