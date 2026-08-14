#include <draw/3d/3d_shapes.h>
#include <maths.h>
#include "../../screens/screensValues.h"

#include "../../textures/textures_types.h"
#include <vector>

static bool initialized3D = false;

#include <pak_loader/pak_loader.h>
#include "../../models3d.h"

#if defined(PLATFORM_PC)
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glu.h>
#elif defined(PLATFORM_3DS)
#include "../shaders/pica200/pica.shader.h"
#include "3dshader_shbin.h"

static P200S_SHADER *shader;

typedef struct
{
    float pos[3];
    float uv[2];
    u8 color[4];
} Vertex3DS;

static std::vector<Vertex3DS> gpuVertices = std::vector<Vertex3DS>(0);
#endif

Vec3 D3D_GetCameraPos(Camera3D *c)
{
    if(!c)
        return {0,0,0};
    return c->pos;
}
Vec3 D3D_GetCameraRot(Camera3D *c)
{
    if(!c)
        return {0,0,0};
    return c->rot;
}
float D3D_GetCameraFov(Camera3D *c)
{
    if(!c)
        return 0;
    return c->fov;
}

void D3D_SetCameraPos(Camera3D *c, Vec3 pos)
{
    c->pos = pos;
}
void D3D_SetCameraRot(Camera3D *c, Vec3 rot)
{
    c->rot = vec3_create(fmodf(rot.x, 360.f), fmodf(rot.y, 360.f), fmodf(rot.z, 360.f));
}
void D3D_SetCameraFov(Camera3D *c, float fov)
{
    c->fov = fov;
}

constexpr float RadToDeg(float rad)
{
    return rad * 180.0f / 3.14159265358979323846f;
}

constexpr float DegToRad(float deg) {
    return deg * 3.14159265358979323846f / 180.0f;
}

Vec3 D3D_LookAt(Vec3 from, Vec3 to)
{
    return D3D_ForwardToRotation(
        vec3_sub(to, from)
    );
}

Vec3 D3D_ForwardToRotation(Vec3 forward)
{
    forward = vec3_normalized(forward);

    Vec3 rot;

    // Pitch
    rot.y = -RadToDeg(asinf(forward.z));

    // Yaw
    rot.z = -RadToDeg(atan2f(forward.y, forward.x));

    // No podemos deducir el roll a partir de un solo vector
    rot.x = 0.0f;

    return rot;
}

// rotation: (pitch=X, yaw=Y, roll=Z) o como lo interpretes
Vec3 D3D_RotationToForward(Vec3 rot)
{
    float yaw   = DegToRad(rot.z);
    float pitch = DegToRad(rot.y);

    Vec3 forward;

    forward.x = cosf(-pitch) * cosf(yaw);
    forward.y = sinf(yaw) * cosf(-pitch);
    forward.z = sinf(-pitch);

    return vec3_normalized(forward);
}

Vec3 D3D_Camera_GetForward(Camera3D *c)
{
    return D3D_RotationToForward(c->rot);
}

Vec3 D3D_Camera_GetRight(Camera3D *c)
{
    Vec3 worldUp = vec3_create(0, 0, 1); // Z up
    return vec3_normalized(vec3_cross(D3D_Camera_GetForward(c), worldUp));
}

Vec3 D3D_Camera_GetUp(Camera3D *c)
{
    return vec3_cross(D3D_Camera_GetRight(c), D3D_Camera_GetForward(c));
}

void D3D_AddForwardPos(Camera3D *c, Vec3 v)
{
    Vec3 forward = D3D_Camera_GetForward(c);
    Vec3 right   = D3D_Camera_GetRight(c);
    Vec3 up      = D3D_Camera_GetUp(c);

    Vec3 move =
        vec3_add(vec3_add(vec3_mul(forward, v.x),
        vec3_mul(right,   v.y)),
        vec3_mul(up,      v.z));

    c->pos = vec3_add(c->pos, move);
}

void D3D_AddFront(Camera3D *c, float amount)
{
    Vec3 forward = D3D_Camera_GetForward(c);
    c->pos = vec3_add(c->pos, vec3_mul(forward, amount));
}

void D3D_AddRight(Camera3D *c, float amount)
{
    Vec3 right = D3D_Camera_GetRight(c);
    c->pos = vec3_add(c->pos, vec3_mul(right, amount));
}

void D3D_AddUp(Camera3D *c, float amount)
{
    c->pos.z += amount;
}

Model3D* loadM3DS(const char* path)
{
    PAK_FILE *f = PAKL_LoadFile(path);
    if (!f)
    {
        printf("No se pudo abrir archivo\n");
        return nullptr;
    }

    // ---------------- HEADER ----------------
    char header[4];
    if (PAKL_fread(header, 1, 4, f) != 4)
    {
        PAKL_CloseFile(f);
        return nullptr;
    }

    if (header[0] != 'M' || header[1] != '3' ||
        header[2] != 'D' || header[3] != 'S')
    {
        printf("Cabecera invalida\n");
        PAKL_CloseFile(f);
        return nullptr;
    }

    uint32_t vcount = 0, fcount = 0;

    PAKL_fread(&vcount, sizeof(uint32_t), 1, f);
    PAKL_fread(&fcount, sizeof(uint32_t), 1, f);

    Model3D* model = new Model3D();

    model->vectorPos.resize(vcount);
    model->faces.resize(fcount);

    // ---------------- VERTICES ----------------
    for (uint32_t i = 0; i < vcount; i++)
    {
        Vec3 v;
        PAKL_fread(&v.x, sizeof(float), 1, f);
        PAKL_fread(&v.y, sizeof(float), 1, f);
        PAKL_fread(&v.z, sizeof(float), 1, f);

        model->vectorPos[i] = v;
    }

    // ---------------- FACES ----------------
    for (uint32_t i = 0; i < fcount; i++)
    {
        Face face = {};

        for (int j = 0; j < 3; j++)
        {
            uint16_t idx;
            float ux, uy;

            PAKL_fread(&idx, sizeof(uint16_t), 1, f);
            PAKL_fread(&ux, sizeof(float), 1, f);
            PAKL_fread(&uy, sizeof(float), 1, f);

            face.vertex[j].vertex = idx;
            face.vertex[j].uv = { ux, uy };
        }

        model->faces[i] = face;
    }

    PAKL_CloseFile(f);

    // ---------------- PRINT DEL MODELO ----------------
    /*
    printf("\n===== MODEL3D DEBUG =====\n");
    printf("Vertices: %u\n", vcount);
    printf("Faces: %u\n\n", fcount);

    for (uint32_t i = 0; i < vcount; i++)
    {
        Vec3 v = model->vectorPos[i];
        printf("V[%u] = (%.4f, %.4f, %.4f)\n",
               i, v.x, v.y, v.z);
    }

    printf("\n--- Faces ---\n");

    for (uint32_t i = 0; i < fcount; i++)
    {
        Face fce = model->faces[i];

        printf("F[%u] = %u/%u  %u/%u  %u/%u\n",
               i,
               fce.vertex[0].vertex,
               0,
               fce.vertex[1].vertex,
               0,
               fce.vertex[2].vertex,
               0);
    }

    printf("=========================\n\n");*/

    printf("Cargado correctamente: %u verts / %u faces\n", vcount, fcount);
    return model;
}

Model3D *D3D_LoadModel3D(const char* path)
{
    return loadM3DS(path);
}

bool D3D_Init()
{
    if(initialized3D)
        return false;

    initialized3D = true;
    models.clear();
    models.resize(2);

    //AddCubeModel(0);
    models[0] = loadM3DS("engine/models/Cube.m3ds");
    models[1] = loadM3DS("engine/models/Sphere.m3ds");
    printf("Loaded m3ds's.\n");

#if defined(PLATFORM_3DS)
    gpuVertices.clear();
    shader = P200S_Create();
    if(!shader)
        return false;

    if(P200S_CompileShader(shader, (u32*)_3dshader_shbin, _3dshader_shbin_size))
    {  
        AttrInfo_Init(&shader->attrInfo);
        AttrInfo_AddLoader(&shader->attrInfo, 0, GPU_FLOAT,         3); // v0=position
        AttrInfo_AddLoader(&shader->attrInfo, 1, GPU_FLOAT,         2); // v1=texcoord
        AttrInfo_AddLoader(&shader->attrInfo, 2, GPU_UNSIGNED_BYTE, 4); // v3=color

        BufInfo_Init(&shader->bufInfo);

        Mtx_OrthoTilt(&shader->s_projTop, 0.0f, 400.0f, 240.0f, 0.0f, 1000.0f, 0.1f, true);
        Mtx_OrthoTilt(&shader->s_projBot, 0.0f, 320.0f, 240.0f, 0.0f, 1000.0f, 0.1f, true);

        // Get uniform locations
        shader->uLoc_mvpMtx =
        shaderInstanceGetUniformLocation(
            shader->program.vertexShader,
            "mvpMtx");

        return true;
    }
    P200S_Destroy(shader);
    shader = nullptr;
    initialized3D = false;
    return false;
#endif
    return true;
}

void D3D_Prepare()
{
    if(!initialized3D)
        return;
#if defined(PLATFORM_PC)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
#elif defined(PLATFORM_3DS)
    P200S_LinkProgram(shader);
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);

	C3D_CullFace(GPU_CULL_BACK_CCW);
#endif
}

#if defined(PLATFORM_3DS)
static void BuildGPUBuffer(Model3D* model, Vec3 scale, Material *mat)
{
    gpuVertices.clear();

    for (u16 i = 0; i < model->faces.size(); i++)
    {
        Face& f = model->faces[i];

        for (int j = 0; j < 3; j++)
        {
            u16 idx = f.vertex[j].vertex;

            Vec3 p = model->vectorPos[idx];
            p = vec3_mul_vec(p, scale);

            Vertex3DS v;

            v.pos[0] = p.x;
            v.pos[1] = p.y;
            v.pos[2] = p.z;

            v.uv[0] = f.vertex[j].uv.x;
            v.uv[1] = f.vertex[j].uv.y;

            v.color[0] = mat->color.r;
            v.color[1] = mat->color.g;
            v.color[2] = mat->color.b;
            v.color[3] = mat->color.a;

            gpuVertices.push_back(v);
        }
    }
}
#endif

#if defined(PLATFORM_PC)
bool DrawFace(VertexPos v1, UV uv1,
              VertexPos v2, UV uv2,
              VertexPos v3, UV uv3)
{
    glBegin(GL_TRIANGLES);

    glTexCoord2f(uv1.x, uv1.y);
    glVertex3f(v1.x, v1.y, v1.z);
    glTexCoord2f(uv2.x, uv2.y);
    glVertex3f(v2.x, v2.y, v2.z);
    glTexCoord2f(uv3.x, uv3.y);
    glVertex3f(v3.x, v3.y, v3.z);

    glEnd();

    return true;
}
#endif

bool DrawModel(Model3D *model, Vec3 position, Vec3 rotation, Vec3 scale, Material *material, Camera3D *camera)
{
    if(!initialized3D)
        return false;
    if(!model || !material || !camera)
    {
        //printf("[DRAW 3D] Error loading model, material or camera.\n");
        return false;
    }
    //printf("[DRAW 3D] Drawing.\n");
    
    u16 size = (u16)model->faces.size();
    
    bool draw = true;

#if defined(PLATFORM_PC)
    glEnable(GL_DEPTH_TEST);
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    glUseProgram(0);

    glEnable(GL_DEPTH_TEST);

    float vx, vy, vw, vh;

    if(currScreen == TOP)
    {
        vx = topInitialPointX;
        vy = topInitialPointY;
        vw = topWidth;
        vh = topHeight;
    }
    else
    {
        float offset = ((float)(SCREEN_TOP_WIDTH - SCREEN_BOT_WIDTH) * windowScale) / 2.0f;

        vx = bottomInitialPointX - offset;
        vy = bottomInitialPointY;
        vw = botWidth + (offset * 2.0f);
        vh = botHeight;
    }

    // OpenGL usa origen abajo-izquierda
    glViewport(
        (int)vx,
        wheight - (int)(vy + vh),
        (int)vw,
        (int)vh
    );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float)SCREEN_TOP_WIDTH / (float)SCREEN_HEIGHT;

    gluPerspective(
        camera->fov,
        aspect,
        0.1f,
        1000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Cámara
    glRotatef(-camera->rot.x, 0,0,1);
    glRotatef(-camera->rot.y, 1,0,0);
    glRotatef(-camera->rot.z, 0,1,0);

    glTranslatef(
        -camera->pos.y,
        -camera->pos.z,
        -camera->pos.x);

    // Modelo
    glPushMatrix();

    glTranslatef(
        position.y,
        position.x,
        position.z);

    glRotatef(rotation.x, 0,0,1);
    glRotatef(rotation.y, 1,0,0);
    glRotatef(rotation.z, 0,1,0);

    glScalef(
        scale.x,
        scale.y,
        scale.z);
        
    glBindTexture(GL_TEXTURE_2D, material->diffusion ? material->diffusion->sheet : 0);
    glColor4ub(255,255,255,255);

    for(u16 i = 0; i < size; i++)
    {
        Vec3 v1 = vec3_mul_vec(model->vectorPos[model->faces[i].vertex[0].vertex], scale);
        Vec3 v2 = vec3_mul_vec(model->vectorPos[model->faces[i].vertex[1].vertex], scale);
        Vec3 v3 = vec3_mul_vec(model->vectorPos[model->faces[i].vertex[2].vertex], scale);

        if(!DrawFace(v1, model->faces[i].vertex[0].uv, 
                     v2, model->faces[i].vertex[1].uv, 
                     v3, model->faces[i].vertex[2].uv))
        {
            draw = false;
            break;
        }
    }

    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, 0);
    glViewport(vp[0], vp[1], vp[2], vp[3]);
#elif defined(PLATFORM_3DS)

    BuildGPUBuffer(model, scale, material);

    C3D_Mtx proj;
    Mtx_PerspTilt(
        &proj,
        DegToRad(camera->fov),
        400.0f / 240.0f,
        0.1f,
        1000.0f,
        false
    );

    C3D_Mtx view;
    Mtx_Identity(&view);

    Mtx_RotateZ(&view, DegToRad(-camera->rot.z), true);
    Mtx_RotateY(&view, DegToRad(-camera->rot.y), true);
    Mtx_RotateX(&view, DegToRad(-camera->rot.x), true);

    Mtx_Translate(
        &view,
        -camera->pos.x,
        -camera->pos.y,
        -camera->pos.z,
        true
    );

    C3D_Mtx modelM;
    Mtx_Identity(&modelM);

    Mtx_Translate(
        &modelM,
        position.x,
        position.y,
        position.z,
        true
    );

    Mtx_RotateX(&modelM, DegToRad(rotation.x), true);
    Mtx_RotateY(&modelM, DegToRad(rotation.y), true);
    Mtx_RotateZ(&modelM, DegToRad(rotation.z), true);

    C3D_Mtx mv;
    Mtx_Multiply(&mv, &view, &modelM);

    C3D_Mtx mvp;
    Mtx_Multiply(&mvp, &proj, &mv);

    GSPGPU_FlushDataCache(
        gpuVertices.data(),
        gpuVertices.size() * sizeof(Vertex3DS)
    );

    BufInfo_Add(
        &shader->bufInfo,
        gpuVertices.data(),
        sizeof(Vertex3DS),
        3,
        0x210
    );

    C3D_TexEnv* env = C3D_GetTexEnv(0);

    C3D_TexEnvInit(env);
    if(material && material->diffusion && material->diffusion->tex)
    {
        C3D_TexBind(0, material->diffusion->tex);

        C3D_TexEnvSrc(
            env,
            C3D_Both,
            GPU_TEXTURE0,
            GPU_PRIMARY_COLOR,
            GPU_PRIMARY_COLOR
        );

        C3D_TexEnvFunc(
            env,
            C3D_Both,
            GPU_MODULATE
        );
    }
    else
    {
        C3D_TexEnvSrc(
            env,
            C3D_Both,
            GPU_PRIMARY_COLOR,
            GPU_PRIMARY_COLOR,
            GPU_PRIMARY_COLOR
        );

        C3D_TexEnvFunc(
            env,
            C3D_Both,
            GPU_REPLACE
        );
    }

    C3D_TexEnvSrc(
        env,
        C3D_Both,
        GPU_TEXTURE0,
        GPU_PRIMARY_COLOR,
        GPU_PRIMARY_COLOR
    );

    C3D_TexEnvFunc(
        env,
        C3D_Both,
        GPU_MODULATE
    );

    shader->mvpMtx = mvp;
    P200S_Use(shader);

    C3D_DrawArrays(
        GPU_TRIANGLES,
        0,
        (int)gpuVertices.size()
    );

    gpuVertices.clear();

#endif

    return draw;
}

bool D3D_DrawModel(Model3D *model, Vec3 position, Vec3 rotation, Vec3 scale, Material material, Camera3D *camera)
{
    return DrawModel(model, position, rotation, scale, &material, camera);
}

bool D3D_DrawCuboid(Vec3 position, Vec3 rotation, Vec3 scale, Vec3 center, Material material, Camera3D *camera)
{
    if(!initialized3D)
        return false;
    if(!camera || !models[0])
        return false;

    center = vec3_clamp_float(center, 0.f, 1.f);

    Vec3 size = vec3_create(2.f * scale.x, 2.f * scale.y, 2.f * scale.z);

    Vec3 offset;
    offset.x = (0.5f - center.x) * size.x;
    offset.y = (0.5f - center.y) * size.y;
    offset.z = (0.5f - center.z) * size.z;

    position.x += offset.x;
    position.y += offset.y;
    position.z += offset.z;

    return DrawModel(models[0], position, rotation, scale, &material, camera);
}
bool D3D_DrawSphere(Vec3 position, Vec3 rotation, Vec3 scale, Vec3 center, Material material, Camera3D *camera)
{
    if(!initialized3D)
        return false;
    if(!camera || !models[1])
        return false;

    center = vec3_clamp_float(center, 0.f, 1.f);

    Vec3 size = vec3_create(2.f * scale.x, 2.f * scale.y, 2.f * scale.z);

    Vec3 offset;
    offset.x = (0.5f - center.x) * size.x;
    offset.y = (0.5f - center.y) * size.y;
    offset.z = (0.5f - center.z) * size.z;

    position.x += offset.x;
    position.y += offset.y;
    position.z += offset.z;

    return DrawModel(models[1], position, rotation, scale, &material, camera);
}

void D3D_Exit()
{
    if(!initialized3D)
        return;
    for(auto *model : models)
    {
        delete model;
    }
    models.clear();
#if defined(PLATFORM_3DS)
    gpuVertices.clear();
    P200S_Destroy(shader);
    shader = nullptr;
#endif
    initialized3D = false;
}