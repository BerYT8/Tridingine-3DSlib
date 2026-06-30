#pragma once
#if defined(PLATFORM_PC)
#define GLEW_STATIC
#include <GL/glew.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct SPOGL_SHADER
{
    GLuint gVertexShader;
    GLuint gFragmentShader;

    GLuint gProgram;
} SPOGL_SHADER;

SPOGL_SHADER *SPOGL_CreateShader();
bool SPOGL_CompileShader(SPOGL_SHADER *shaderStruct, const char* shader);
bool SPOGL_CompileFragmentShader(SPOGL_SHADER *shaderStruct, const char* shader);
bool SPOGL_LinkProgram(SPOGL_SHADER *shaderStruct);
bool SPOGL_Use(SPOGL_SHADER *shaderStruct);
bool SPOGL_Destroy(SPOGL_SHADER *shaderStruct);

#ifdef __cplusplus
}
#endif

#endif