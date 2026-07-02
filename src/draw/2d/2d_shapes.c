#include <draw/2d/2d_shapes.h>

#include <maths.h>

#if defined(PLATFORM_PC)
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include "../shaders/opengl/opengl.shader.h"


const char *vertexShader2D = "\
#version 120 // <- Cambiado a compatibility\n\
\n\
// Ya no necesitas layouts ni uniforms manuales,\n\
// OpenGL rellena las variables integradas automáticamente.\n\
\n\
out vec4 oColor;\n\
\n\
void main()\n\
{\n\
    // gl_ModelViewProjectionMatrix junta tus glTranslate/glRotate y proyecciones\n\
    // gl_Vertex lee directamente tus glVertex2f\n\
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n\
\n\
    // gl_Color lee directamente tus glColor4ub\n\
    oColor = gl_Color; \n\
    \n\
    // Si necesitas texturas con glBegin, usarías gl_MultiTexCoord0\n\
}\n\
";

const char* fragmentShader2D = "\
#version 120\n\
\n\
in vec2 oTexCoord0;\n\
in vec2 oTexCoord1;\n\
in vec4 oColor;\n\
\n\
out vec4 FragColor;\n\
\n\
void main()\n\
{\n\
    FragColor = oColor;\n\
}\n\
";

const char *vertexShaderEllipse2D = "\
#version 120\n\
\n\
out vec2 vLocal;\n\
out vec4 vColor;\n\
\n\
void main()\n\
{\n\
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n\
    vColor = gl_Color;\n\
\n\
    // Convierte UV (0..1) -> (-1..1)\n\
    vLocal = gl_MultiTexCoord0.xy * 2.0 - 1.0;\n\
}\n\
";

const char* fragmentShaderEllipse2D = "\
#version 120\n\
\n\
in vec2 vLocal;\n\
in vec4 vColor;\n\
\n\
out vec4 FragColor;\n\
\n\
void main()\n\
{\n\
    if(dot(vLocal, vLocal) > 1.0)\n\
        discard;\n\
\n\
    FragColor = vColor;\n\
}\n\
";

const char *vertexShaderQuad2D = "\
#version 120\n\
\n\
out vec2 vUV;\n\
\n\
void main()\n\
{\n\
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n\
    vUV = gl_MultiTexCoord0.xy;\n\
}\n\
";

const char* fragmentShaderQuad2D = "\
#version 120\n\
\n\
uniform vec4 c1;\n\
uniform vec4 c2;\n\
uniform vec4 c3;\n\
uniform vec4 c4;\n\
\n\
in vec2 vUV;\n\
\n\
out vec4 FragColor;\n\
\n\
const float GAMMA = 1.3;\n\
\n\
vec3 ToLinear(vec3 c)\n\
{\n\
    return pow(c, vec3(GAMMA));\n\
}\n\
\n\
vec3 ToSRGB(vec3 c)\n\
{\n\
    return pow(c, vec3(1.0/GAMMA));\n\
}\n\
\n\
void main()\n\
{\n\
    float triangleBlend = 0.2;\n\
    float colorStrength = 1.4;\n\
    float u = clamp(vUV.x,0.0,1.0);\n\
    float v = clamp(vUV.y,0.0,1.0);\n\
\n\
    vec3 a = mix(ToLinear(c1.rgb), ToLinear(c2.rgb), u);\n\
    vec3 b = mix(ToLinear(c4.rgb), ToLinear(c3.rgb), u);\n\
    vec3 bilinear = mix(a, b, v);\n\
\n\
    // Interpolación como si fueran dos triángulos\n\
    vec3 triangle;\n\
\n\
    if (u + v <= 1.0)\n\
    {\n\
        // Triángulo c1-c2-c4\n\
        triangle =\n\
            ToLinear(c1.rgb) * (1.0 - u - v)\n\
            + ToLinear(c2.rgb) * u\n\
            + ToLinear(c4.rgb) * v;\n\
    }\n\
    else\n\
    {\n\
        // Triángulo c2-c3-c4\n\
        triangle =\n\
            ToLinear(c3.rgb) * (u + v - 1.0)\n\
            + ToLinear(c2.rgb) * (1.0 - v)\n\
            + ToLinear(c4.rgb) * (1.0 - u);\n\
    }\n\
\n\
    vec3 rgb = mix(bilinear, triangle, triangleBlend);\n\
\n\
    rgb *= colorStrength;\n\
    FragColor.rgb = ToSRGB(rgb);\n\
    FragColor.a = mix(mix(c1.a,c2.a,u), mix(c4.a,c3.a,u), v);\n\
}\n\
";

static SPOGL_SHADER *shader2D = NULL;
static SPOGL_SHADER *shaderEllipse = NULL;
static SPOGL_SHADER *shaderQuad = NULL;

#elif defined(PLATFORM_3DS)
#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#endif
#include "../../screens/screensValues.h"
#define ALLOCATE_SHMEM
#include "2d_vals.h"

bool D2D_Init()
{
    if(!screensInitialized)
        return false;
    if(initialized)
        return false;
    initialized = true;
#if defined(PLATFORM_PC)
    shader2D = SPOGL_CreateShader();
    if(!SPOGL_CompileShader(shader2D, vertexShader2D) || !SPOGL_CompileFragmentShader(shader2D, fragmentShader2D))
    {    
        printf("[ERROR] Error compiling shader2D.\n");
        SPOGL_Destroy(shader2D);
        shader2D = NULL;

        initialized = false;
        return false;
    }
    shaderEllipse = SPOGL_CreateShader();
    if(!SPOGL_CompileShader(shaderEllipse, vertexShaderEllipse2D) || !SPOGL_CompileFragmentShader(shaderEllipse, fragmentShaderEllipse2D))
    {    
        printf("[ERROR] Error compiling shaderEllipse.\n");
        SPOGL_Destroy(shaderEllipse);
        shaderEllipse = NULL;

        SPOGL_Destroy(shader2D);
        shader2D = NULL;
        initialized = false;
        return false;
    }
    shaderQuad = SPOGL_CreateShader();
    if(!SPOGL_CompileShader(shaderQuad, vertexShaderQuad2D) || !SPOGL_CompileFragmentShader(shaderQuad, fragmentShaderQuad2D))
    {    
        printf("[ERROR] Error compiling shaderQuad.\n");
        SPOGL_Destroy(shaderQuad);
        shaderQuad = NULL;

        SPOGL_Destroy(shader2D);
        shader2D = NULL;
        SPOGL_Destroy(shaderEllipse);
        shaderEllipse = NULL;
        initialized = false;
        return false;
    }
    //glDisable(GL_FRAMEBUFFER_SRGB);
    TTF_Init();
    return true;
#elif defined(PLATFORM_3DS)
    return C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
#endif
    return true;
}

void D2D_Prepare()
{
    if(!initialized)
        return;
#if defined(PLATFORM_PC)
    SPOGL_LinkProgram(shader2D);
    SPOGL_LinkProgram(shaderEllipse);
    SPOGL_LinkProgram(shaderQuad);
#elif defined(PLATFORM_3DS)
    C2D_Prepare();
#endif
}

float D2D_ValueIsRotation(float value)
{
    float totalValue = fmodf(value, 360.0f);

    if (totalValue < 0.0f)
    {
        totalValue += 360.0f;
    }

    return totalValue;
}

void D2D_AddRotation(float *value, float rotation)
{
    if(!value)
        return;

    // 1. Calcular el valor total sumando la rotación actual
    float totalValue = *value + rotation;

    // 2. Aplicar el módulo flotante para reducirlo al rango (-360, 360)
    totalValue = fmodf(totalValue, 360.0f);

    // 3. Si el resultado es negativo, sumar 360 para pasarlo al rango positivo [0, 360)
    if (totalValue < 0.0f)
    {
        totalValue += 360.0f;
    }

    *value = totalValue;
}

D2D_Result D2D_DrawPoint(float x, float y, float rotation, float depth, float thickness, Color color)
{
/*
#if defined(PLATFORM_PC)
    SPOGL_Use(shader);
    glPushMatrix();
    glTranslatef(, 0, depth);


    glPointSize(thickness);

    glBegin(GL_POINTS);

    glColor4ub(color.r, color.g, color.b, color.a);
    glVertex2f(x, y);

    glEnd();

    glPopMatrix();
#elif defined(PLATFORM_3DS)
    return C2D_DrawLine(x, y, Color_ToUInt32_Default(color), x, y, Color_ToUInt32_Default(color), thickness, depth);
#endif
    return D2D_OK;
*/
    return D2D_DrawRectSolid(x, y, thickness, thickness, rotation, depth, 0.5f, 0.5f, color);
}

D2D_Result D2D_DrawLine(float x0, float y0, Color c0,
                    float x1, float y1, Color c1,
                    float thickness, float rotation, float depth, float align)
{
    if(!initialized)
        return D2D_NOT_INITIALIZED;

    if(depth < -1 || depth > 1)
        return D2D_INVALID_ARGUMENT;

    if(!isValidScreen())
        return D2D_ERROR;

    align = clampf(align, 0.f, 1.f);
    float pivotX = x0 + (x1 - x0) * align;
    float pivotY = y0 + (y1 - y0) * align;
#if defined(PLATFORM_PC)
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    SPOGL_Use(shader2D);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(
        0,
        wwidth,
        wheight,
        0,
        -1,
        1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
    glTranslatef(pivotX, pivotY, depth);
    glRotatef(rotation, 0.f, 0.f, 1.f);

    glLineWidth(thickness);

    glBegin(GL_LINES);

    glColor4ub(c0.r, c0.g, c0.b, c0.a);
    glVertex2f(x0 - pivotX, y0 - pivotY);

    glColor4ub(c1.r, c1.g, c1.b, c1.a);
    glVertex2f(x1 - pivotX, y1 - pivotY);

    glEnd();

    glPopMatrix();
#elif defined(PLATFORM_3DS)
    C3D_Mtx mtx;
    C2D_ViewSave(&mtx);
    C2D_ViewTranslate(pivotX, pivotY);
    C2D_ViewRotateDegrees(rotation);
    bool r = C2D_DrawLine(x0 - pivotX, y0 - pivotY, Color_ToUInt32_Default(c0), x1 - pivotX, y1 - pivotY, Color_ToUInt32_Default(c1), thickness, depth);
    C2D_ViewRestore(&mtx);
    return r;
#endif
    return D2D_OK;
}

D2D_Result D2D_DrawRectangle(float x, float y, float w, float h, float rotation, float depth, float alignX, float alignY, Color c1, Color c2, Color c3, Color c4)
{
    if(!initialized)
        return D2D_NOT_INITIALIZED;

    if(depth < -1 || depth > 1)
        return D2D_INVALID_ARGUMENT;

    if(!isValidScreen())
        return D2D_ERROR;

#if defined(PLATFORM_PC)
    w *= windowScale;
    h *= windowScale;
    x *= windowScale;
    y *= windowScale;

    x += (currScreen == TOP ? topInitialPointX : bottomInitialPointX);
    y += (currScreen == TOP ? topInitialPointY : bottomInitialPointY);
#endif

    alignX = clampf(alignX, 0.f, 1.f);
    alignY = clampf(alignY, 0.f, 1.f);

#if defined(PLATFORM_PC)
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(
        0,
        wwidth,
        wheight,
        0,
        -1,
        1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    SPOGL_Use(shaderQuad);

    glUniform4f(glGetUniformLocation(shaderQuad->gProgram, "c1"),
            c1.r/255.f, c1.g/255.f, c1.b/255.f, c1.a/255.f);

    glUniform4f(glGetUniformLocation(shaderQuad->gProgram, "c2"),
                c2.r/255.f, c2.g/255.f, c2.b/255.f, c2.a/255.f);

    glUniform4f(glGetUniformLocation(shaderQuad->gProgram, "c3"),
                c3.r/255.f, c3.g/255.f, c3.b/255.f, c3.a/255.f);

    glUniform4f(glGetUniformLocation(shaderQuad->gProgram, "c4"),
                c4.r/255.f, c4.g/255.f, c4.b/255.f, c4.a/255.f);
    //glUniform1f(glGetUniformLocation(shaderQuad->gProgram, "triangleBlend"), colorStrength);
    //glUniform1f(glGetUniformLocation(shaderQuad->gProgram, "GAMMA"), gamma);

    float pivotOffsetX = w * 0.5f - w * alignX;
    float pivotOffsetY = h * 0.5f - h * alignY;

    glPushMatrix();

    glTranslatef(x, y, depth);
    glRotatef(rotation, 0.f, 0.f, 1.f);
    glTranslatef(pivotOffsetX, pivotOffsetY, 0.f);

    glBegin(GL_TRIANGLES);

    // TL
    //glColor4ub(c1.r,c1.g,c1.b,c1.a);
    glTexCoord2f(0.f,0.f);
    glVertex2f(-w*0.5f,-h*0.5f);

    // BL
    //glColor4ub(c4.r,c4.g,c4.b,c4.a);
    glTexCoord2f(0.f,1.f);
    glVertex2f(-w*0.5f, h*0.5f);

    // TR
    //glColor4ub(c2.r,c2.g,c2.b,c2.a);
    glTexCoord2f(1.f,0.f);
    glVertex2f( w*0.5f,-h*0.5f);

    // TR
    //glColor4ub(c2.r,c2.g,c2.b,c2.a);
    glTexCoord2f(1.f,0.f);
    glVertex2f( w*0.5f,-h*0.5f);

    // BL
    //glColor4ub(c4.r,c4.g,c4.b,c4.a);
    glTexCoord2f(0.f,1.f);
    glVertex2f(-w*0.5f, h*0.5f);

    // BR
    //glColor4ub(c3.r,c3.g,c3.b,c3.a);
    glTexCoord2f(1.f,1.f);
    glVertex2f( w*0.5f, h*0.5f);

    glEnd();

    glPopMatrix();

#elif defined(PLATFORM_3DS)
    C3D_Mtx mtx;
    C2D_ViewSave(&mtx);
    
    // Hacemos lo mismo en 3DS: trasladar a (x, y) y rotar allí
    C2D_ViewTranslate(x, y);
    C2D_ViewRotateDegrees(rotation);
    
    // El rectángulo local se dibuja compensando su alineamiento relativo al origen (0,0)
    bool r = C2D_DrawRectangle(-w * alignX, -h * alignY, depth, w, h, Color_ToUInt32_Default(c1), Color_ToUInt32_Default(c2), Color_ToUInt32_Default(c4), Color_ToUInt32_Default(c3));
    C2D_ViewRestore(&mtx);
    return r;
#endif
    return D2D_OK;
}

/*
#if defined(PLATFORM_PC)

void drawArc(float x, float y, float radius,
             float startAngle, float endAngle, int segments, float depth,
             Color color)
{
    glColor4ub(color.r, color.g, color.b, color.a);
    float cx = x + radius;
    float cy = y + radius;

    for (int i = 0; i <= segments; i++) {
        float a0 = (M_PI_2 * i) / segments;
        float a1 = (M_PI_2 * (i + 1)) / segments;

        D2D_DrawLine(cx + cos(a0) * radius, cy + sin(a0) * radius, color, cx + cos(a1) * radius, cy + sin(a1) * radius, color, 1.0f, 0.f, depth, 0.f);
    }
}
#elif defined(PLATFORM_3DS)

void drawArc(float x, float y, float radius,
             float startAngle, float endAngle, int segments, float depth,
             Color color)
{
    float step = (endAngle - startAngle) / segments;

    float cx = x + radius;
    float cy = y + radius;

    for (int i = 0; i < segments; i++)
    {
        float a1 = startAngle + step * i;
        float a2 = startAngle + step * (i + 1);

        float x1 = cx + cosf(a1) * radius;
        float y1 = cy + sinf(a1) * radius;

        float x2 = cx + cosf(a2) * radius;
        float y2 = cy + sinf(a2) * radius;

        C2D_DrawLine(x1, y1, Color_ToUInt32_Default(color), x2, y2, Color_ToUInt32_Default(color), 2.0f, 0.0f);
    }
}

#endif
*/

D2D_Result D2D_DrawBorderedRect(float x, float y, float w, float h, float radius, float depth, float alignX, float alignY, Color c1, Color c2, Color c3, Color c4)
{
    if(!initialized)
        return D2D_NOT_INITIALIZED;

    if(depth < -1 || depth > 1)
        return D2D_INVALID_ARGUMENT;

    if(!isValidScreen())
        return D2D_ERROR;
    
    float offset = 5.0f;
    radius = clampf(radius, 0.0f, min2(w, h)/2.0f - offset);
        
    alignX = clampf(alignX, 0.f, 1.f);
    alignY = clampf(alignY, 0.f, 1.f);

    float pivotOffsetX = -w * alignX;
    float pivotOffsetY = -h * alignY;
    
#if defined(PLATFORM_PC)
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(
        0,
        wwidth,
        wheight,
        0,
        -1,
        1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Traslación base a la pantalla en PC
    int screenStartX = (currScreen == TOP ? topInitialPointX : bottomInitialPointX);
    int screenStartY = (currScreen == TOP ? topInitialPointY : bottomInitialPointY);

    SPOGL_Use(shader2D);
    glPushMatrix();

    // Escalamos y trasladamos el origen local al pivote del rectángulo
    glTranslatef((x * windowScale) + screenStartX, (y * windowScale) + screenStartY, depth);
    //glRotatef(rotation, 0.f, 0.f, 1.f);
    glScalef(windowScale, windowScale, 1.0f);

    // --- DIBUJAR CUADRADOS DE LA BASE ---
    glBegin(GL_QUADS);
    glColor4ub(c1.r, c1.g, c1.b, c1.a);

    // Cuadrado superior
    glVertex2f(pivotOffsetX + radius, pivotOffsetY);
    glVertex2f(pivotOffsetX + radius, pivotOffsetY + radius);
    glVertex2f(pivotOffsetX + w - radius, pivotOffsetY + radius);
    glVertex2f(pivotOffsetX + w - radius, pivotOffsetY);

    // Cuadrado inferior (Corregida la altura h - radius)
    glVertex2f(pivotOffsetX + radius, pivotOffsetY + h - radius);
    glVertex2f(pivotOffsetX + radius, pivotOffsetY + h);
    glVertex2f(pivotOffsetX + w - radius, pivotOffsetY + h);
    glVertex2f(pivotOffsetX + w - radius, pivotOffsetY + h - radius);

    // Cuadrado medio central
    glVertex2f(pivotOffsetX, pivotOffsetY + radius);
    glVertex2f(pivotOffsetX + w, pivotOffsetY + radius);
    glVertex2f(pivotOffsetX + w, pivotOffsetY + h - radius);
    glVertex2f(pivotOffsetX, pivotOffsetY + h - radius);
    glEnd();

    // --- DIBUJAR CÍRCULOS BAJO LA MATRIZ DE ROTACIÓN ---
    // Nota: Pasamos coordenadas relativas al origen rotado.
    // Usamos variables auxiliares para calcular las regiones de tijera relativas (requieren lógica de PC si se usa)
    // Para simplificar y mantener la consistencia con tu lógica de regiones en PC:
    glPopMatrix();

    // Dibujar círculos en PC (Usando la lógica original pero corrigiendo posiciones globales en base a windowScale)
    float sc = windowScale;
    
    // Top-Left
    setDrawRegion(x + pivotOffsetX, y + pivotOffsetY, radius + offset, radius + offset);
    D2D_DrawCircleSolid(x + pivotOffsetX, y + pivotOffsetY, radius, 0, depth, 0, 0, c1);

    // Top-Right
    setDrawRegion(x + pivotOffsetX + w - radius - offset, y + pivotOffsetY, radius + offset, radius + offset);
    D2D_DrawCircleSolid(x + pivotOffsetX + w - radius*2.0f, y + pivotOffsetY, radius, 0, depth, 0, 0, c1);
    
    // Bottom-Left
    setDrawRegion(x + pivotOffsetX, y + pivotOffsetY + h - radius - offset, radius + offset, radius + offset);
    D2D_DrawCircleSolid(x + pivotOffsetX, y + pivotOffsetY + h - radius*2.0f, radius, 0, depth, 0, 0, c1);

    // Bottom-Right
    setDrawRegion(x + pivotOffsetX + w - radius - offset, y + pivotOffsetY + h - radius - offset, radius + offset, radius + offset);
    D2D_DrawCircleSolid(x + pivotOffsetX + w - radius*2.0f, y + pivotOffsetY + h - radius*2.0f, radius, 0, depth, 0, 0, c1);
    
    stopDrawRegion();

#elif defined(PLATFORM_3DS)

    C3D_Mtx mtx;
    C2D_ViewSave(&mtx);
    C2D_ViewTranslate(x, y);
    //C2D_ViewRotateDegrees(rotation);

    // --- DIBUJAR CÍRCULOS (Ajustados según el comportamiento de la esquina inferior derecha) ---
    
    // Esquina Superior Izquierda: Desplazada hacia la derecha y hacia abajo una distancia de un diámetro
    setDrawRegion(pivotOffsetX, pivotOffsetY, radius, radius);
    bool dc1 = D2D_DrawCircleSolid(pivotOffsetX, pivotOffsetY, radius, 0, depth, 0, 0, c1);
    stopDrawRegion();

    // Esquina Superior Derecha: Desplazada hacia abajo una distancia de un diámetro
    setDrawRegion(pivotOffsetX + w - radius, pivotOffsetY, radius, radius);
    bool dc2 = D2D_DrawCircleSolid(pivotOffsetX + w - radius*2.0f, pivotOffsetY, radius, 0, depth, 0, 0, c1);
    stopDrawRegion();

    // Esquina Inferior Izquierda: Desplazada hacia la derecha una distancia de un diámetro
    setDrawRegion(pivotOffsetX, pivotOffsetY + h - radius, radius, radius);
    bool dc3 = D2D_DrawCircleSolid(pivotOffsetX, pivotOffsetY + h - radius*2.0f, radius, 0, depth, 0, 0, c1);
    stopDrawRegion();

    // Esquina Inferior Derecha (La que ya funcionaba bien): Se queda exactamente igual
    setDrawRegion(pivotOffsetX + w - radius, pivotOffsetY + h - radius, radius, radius);
    bool dc4 = D2D_DrawCircleSolid(pivotOffsetX + w - radius*2.0f, pivotOffsetY + h - radius*2.0f, radius, 0, depth, 0, 0, c1);
    stopDrawRegion();

    // --- DIBUJAR RECTÁNGULOS ---
    bool drt = D2D_DrawRectangle(pivotOffsetX + radius, pivotOffsetY, w - radius*2.0f, radius, 0, depth, 0, 0, c1, c1, c1, c1);
    bool drc = D2D_DrawRectangle(pivotOffsetX, pivotOffsetY + radius, w, h - radius*2.0f, 0, depth, 0, 0, c1, c1, c1, c1);
    bool drb = D2D_DrawRectangle(pivotOffsetX + radius, pivotOffsetY + h - radius, w - radius*2.0f, radius, 0, depth, 0, 0, c1, c1, c1, c1);

    C2D_ViewRestore(&mtx);

    return dc1 && dc2 && dc3 && dc4 && drt && drc && drb;
#endif
    
    return D2D_OK;
}


D2D_Result D2D_DrawEllipse(float x, float y, float radiusX, float radiusY, float rotation, float depth, float alignX, float alignY, Color c0, Color c1, Color c2, Color c3)
{
    if(!initialized)
        return D2D_NOT_INITIALIZED;

    if(depth < -1 || depth > 1)
        return D2D_INVALID_ARGUMENT;

    if(!isValidScreen())
        return D2D_ERROR;

    alignX = clampf(alignX, 0.f, 1.f);
    alignY = clampf(alignY, 0.f, 1.f);

#if defined(PLATFORM_PC)
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(
        0,
        wwidth,
        wheight,
        0,
        -1,
        1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glLoadIdentity();
    radiusX *= windowScale;
    radiusY *= windowScale;

    x *= windowScale;
    y *= windowScale;

    float pivotOffsetX = radiusX - radiusX * 2.f * alignX;
    float pivotOffsetY = radiusY - radiusY * 2.f * alignY;

    x += (currScreen == TOP ? topInitialPointX : bottomInitialPointX);
    y += (currScreen == TOP ? topInitialPointY : bottomInitialPointY);

    /*int segmentos = 100;
    SPOGL_Use(shader2D);
    glPushMatrix();

    glTranslatef(x, y, depth);

    glRotatef(rotation, 0.f, 0.f, 1.f);

    glTranslatef(pivotOffsetX, pivotOffsetY, 0.f);

    glBegin(GL_TRIANGLE_FAN);

    AplicarColorBilineal(c0, c1, c2, c3, 0.5f, 0.5f);
    glVertex2f(0, 0);

    for (int i = 0; i <= segmentos; i++) { 
        float angulo = 2.0f * M_PI * (float)i / (float)segmentos;
        float tx = radiusX * cosf(angulo);
        float ty = radiusY * sinf(angulo);

        float u = (tx + radiusX) / (radiusX * 2.0f);
        float v = (ty + radiusY) / (radiusY * 2.0f);

        u = clampf(u, 0.f, 1.f);
        v = clampf(v, 0.f, 1.f);

        AplicarColorBilineal(c0, c1, c2, c3, u, v);
        glVertex2f(tx, ty);
    }
    glEnd();
    glPopMatrix();*/
    SPOGL_Use(shaderEllipse);

    glPushMatrix();

    glTranslatef(x, y, depth);
    glRotatef(rotation, 0.f, 0.f, 1.f);
    glTranslatef(pivotOffsetX, pivotOffsetY, 0.f);

    glBegin(GL_TRIANGLES);

    // Triángulo 1
    glColor4ub(c0.r,c0.g,c0.b,c0.a);
    glTexCoord2f(0.f,0.f);
    glVertex2f(-radiusX,-radiusY);

    glColor4ub(c2.r,c2.g,c2.b,c2.a);
    glTexCoord2f(0.f,1.f);
    glVertex2f(-radiusX, radiusY);

    glColor4ub(c1.r,c1.g,c1.b,c1.a);
    glTexCoord2f(1.f,0.f);
    glVertex2f( radiusX,-radiusY);


    // Triángulo 2
    glColor4ub(c1.r,c1.g,c1.b,c1.a);
    glTexCoord2f(1.f,0.f);
    glVertex2f( radiusX,-radiusY);

    glColor4ub(c2.r,c2.g,c2.b,c2.a);
    glTexCoord2f(0.f,1.f);
    glVertex2f(-radiusX, radiusY);

    glColor4ub(c3.r,c3.g,c3.b,c3.a);
    glTexCoord2f(1.f,1.f);
    glVertex2f( radiusX, radiusY);

    glEnd();

    glPopMatrix();

#elif defined(PLATFORM_3DS)
    float w = radiusX*2;
    float h = radiusY*2;
    //float pivotX = x - w*alignX;
    //float pivotY = y - h*alignY;
    C3D_Mtx mtx;
    C2D_ViewSave(&mtx);
    C2D_ViewTranslate(x, y);
    C2D_ViewRotateDegrees(rotation);
    bool r = C2D_DrawEllipse(-w*alignX, -h*alignY, depth, w, h, Color_ToUInt32_Default(c0), Color_ToUInt32_Default(c1), Color_ToUInt32_Default(c2), Color_ToUInt32_Default(c3));
    C2D_ViewRestore(&mtx);
    return r;
#endif
    return D2D_OK;
}

D2D_Result D2D_DrawTriangle(float x0, float y0, Color c0,
                        float x1, float y1, Color c1,
                        float x2, float y2, Color c2,
                        float rotation, float depth, float alignX, float alignY)
{
    if(!initialized)
        return D2D_NOT_INITIALIZED;

    if(depth < -1 || depth > 1)
        return D2D_INVALID_ARGUMENT;

    if(!isValidScreen())
        return D2D_ERROR;

#if defined(PLATFORM_PC)
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(
        0,
        wwidth,
        wheight,
        0,
        -1,
        1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glLoadIdentity();
    // 1. Escalar y trasladar las coordenadas según la pantalla actual
    x0 *= windowScale; y0 *= windowScale;
    x0 += (currScreen == TOP ? topInitialPointX : bottomInitialPointX);
    y0 += (currScreen == TOP ? topInitialPointY : bottomInitialPointY);

    x1 *= windowScale; y1 *= windowScale;
    x1 += (currScreen == TOP ? topInitialPointX : bottomInitialPointX);
    y1 += (currScreen == TOP ? topInitialPointY : bottomInitialPointY);
    
    x2 *= windowScale; y2 *= windowScale;
    x2 += (currScreen == TOP ? topInitialPointX : bottomInitialPointX);
    y2 += (currScreen == TOP ? topInitialPointY : bottomInitialPointY);
#endif
        
    alignX = clampf(alignX, 0.f, 1.f);
    alignY = clampf(alignY, 0.f, 1.f);

    // 2. Calcular los límites (Bounding Box) del triángulo
    float minX = min3(x0, x1, x2);
    float maxX = max3(x0, x1, x2);
    float minY = min3(y0, y1, y2);
    float maxY = max3(y0, y1, y2);

    float width  = maxX - minX;
    float height = maxY - minY;

    // 3. El origen/pivote del sistema de coordenadas se sitúa en el punto alineado
    float pivotX = minX + width * alignX;
    float pivotY = minY + height * alignY;

#if defined(PLATFORM_PC)
    SPOGL_Use(shader2D);
    glPushMatrix();

    // Trasladamos la matriz al pivote calculado y rotamos allí
    glTranslatef(pivotX, pivotY, depth);
    glRotatef(rotation, 0.f, 0.f, 1.f);

    glBegin(GL_TRIANGLES);

    // 4. Los vértices se dibujan en relación a minX/minY restando el desfase del alineamiento completo
    // De este modo, al cambiar el align, el triángulo se desplaza físicamente en el espacio
    float offsetX = width * alignX;
    float offsetY = height * alignY;

    glColor4ub(c0.r, c0.g, c0.b, c0.a);
    glVertex2f((x0 - minX) - offsetX, (y0 - minY) - offsetY);

    glColor4ub(c1.r, c1.g, c1.b, c1.a);
    glVertex2f((x1 - minX) - offsetX, (y1 - minY) - offsetY);

    glColor4ub(c2.r, c2.g, c2.b, c2.a);
    glVertex2f((x2 - minX) - offsetX, (y2 - minY) - offsetY);

    glEnd();
    glPopMatrix();

#elif defined(PLATFORM_3DS)
    C3D_Mtx mtx;
    C2D_ViewSave(&mtx);
    C2D_ViewTranslate(pivotX, pivotY);
    C2D_ViewRotateDegrees(rotation);
    
    float offsetX = width * alignX;
    float offsetY = height * alignY;

    bool r = C2D_DrawTriangle(
        (x0 - minX) - offsetX, (y0 - minY) - offsetY, Color_ToUInt32_Default(c0), 
        (x1 - minX) - offsetX, (y1 - minY) - offsetY, Color_ToUInt32_Default(c1), 
        (x2 - minX) - offsetX, (y2 - minY) - offsetY, Color_ToUInt32_Default(c2), 
        depth
    );
    C2D_ViewRestore(&mtx);
    return r;
#endif
    return D2D_OK;
}

void D2D_Exit()
{
    if(!initialized)
        return;
    initialized = false;
#if defined(PLATFORM_PC)
    SPOGL_Destroy(shader2D);
    shader2D = NULL;
    SPOGL_Destroy(shaderEllipse);
    shaderEllipse = NULL;
    SPOGL_Destroy(shaderQuad);
    shaderQuad = NULL;
    TTF_Quit();
#elif defined(PLATFORM_3DS)
    C2D_Fini();
#endif
}