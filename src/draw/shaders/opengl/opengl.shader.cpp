#if defined(PLATFORM_PC)

#include "opengl.shader.h"
#include <iostream>

SPOGL_SHADER *SPOGL_CreateShader()
{
    return new SPOGL_SHADER();
}

bool SPOGL_CompileShader(SPOGL_SHADER *shaderStruct, const char* shader)
{
    if(!shaderStruct)
        return false;
    shaderStruct->gVertexShader = glCreateShader(GL_VERTEX_SHADER);
    
    if(!shaderStruct->gVertexShader)
        return false;

    glShaderSource(shaderStruct->gVertexShader, 1, &shader, nullptr);
    glCompileShader(shaderStruct->gVertexShader);

    GLint success;
    glGetShaderiv(shaderStruct->gVertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char log[1024];
        glGetShaderInfoLog(shaderStruct->gVertexShader, sizeof(log), nullptr, log);

        std::cout << log << std::endl;
        return false;
    }

    return true;
}

bool SPOGL_CompileFragmentShader(SPOGL_SHADER *shaderStruct, const char* shader)
{
    if(!shaderStruct)
        return false;
    shaderStruct->gFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    if(!shaderStruct->gFragmentShader)
        return false;

    glShaderSource(shaderStruct->gFragmentShader, 1, &shader, nullptr);
    glCompileShader(shaderStruct->gFragmentShader);

    GLint success;
    glGetShaderiv(shaderStruct->gFragmentShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char log[1024];
        glGetShaderInfoLog(shaderStruct->gFragmentShader, sizeof(log), nullptr, log);

        std::cout << log << std::endl;
        return false;
    }

    return true;
}

bool SPOGL_LinkProgram(SPOGL_SHADER *shaderStruct)
{
    if(!shaderStruct)
        return false;
    shaderStruct->gProgram = glCreateProgram();

    if(!shaderStruct->gProgram)
        return false;

    if(shaderStruct->gVertexShader)
        glAttachShader(shaderStruct->gProgram, shaderStruct->gVertexShader);
    if(shaderStruct->gFragmentShader)
        glAttachShader(shaderStruct->gProgram, shaderStruct->gFragmentShader);

    glLinkProgram(shaderStruct->gProgram);

    GLint success;
    glGetProgramiv(shaderStruct->gProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        char log[1024];
        glGetProgramInfoLog(shaderStruct->gProgram, sizeof(log), nullptr, log);

        std::cout << log << std::endl;
        return false;
    }

    return true;
}

bool SPOGL_Use(SPOGL_SHADER *shaderStruct)
{
    if(!shaderStruct)
        return false;
    if (shaderStruct->gProgram)
        glUseProgram(shaderStruct->gProgram);
    else
        return false;
    return true;
}

bool SPOGL_Destroy(SPOGL_SHADER *shaderStruct)
{
    if(!shaderStruct)
        return false;
    if (shaderStruct->gProgram)
        glDeleteProgram(shaderStruct->gProgram);

    if (shaderStruct->gVertexShader)
        glDeleteShader(shaderStruct->gVertexShader);

    if (shaderStruct->gFragmentShader)
        glDeleteShader(shaderStruct->gFragmentShader);

    delete shaderStruct;
    return true;
}


#endif