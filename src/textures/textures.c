#include "textures_types.h"
#include "json.h"
#include <textures/t3da_types.h>

#include "../screens/screensValues.h"

#include <stdbool.h>
#include <maths.h>
#include <stdlib.h>
#include <string.h>

#include <pak_loader/pak_loader.h>

#include "../romfs_path.h"

#if defined(PLATFORM_PC)

#include <SDL.h>
#include <SDL_image.h>

#include <GL/glew.h>

#include "../draw/shaders/opengl/opengl.shader.h"

SPOGL_SHADER *shader;

#elif defined(PLATFORM_3DS)

#include "sprites.h"
#include <string.h>
#include <3ds.h>
#include <citro2d.h>
#include <tex3ds.h>


#pragma pack(push, 1)
typedef struct TEX3DSHeader
{
    char magic[4]; // "T3DS"

    uint32_t width;
    uint32_t height;

    uint8_t tiles;
} TEX3DSHeader;
#pragma pack(pop)

//static_assert(sizeof(TEX3DSHeader) == 32,
//              "TEX3DSHeader must be 32 bytes");

#endif

u16 sW, sH;
// float aW, aH;

u32 calculate_hash(const char* str)
{
    // Valores constantes oficiales para FNV-1a de 32 bits
    const u32 FNV_OFFSET_BASIS = 2166136261U;
    const u32 FNV_PRIME = 16777619U;
    
    u32 hash = FNV_OFFSET_BASIS;
    
    // Si el puntero es NULL, devolvemos un valor base o un error seguro
    if (str == NULL) {
        return 0;
    }
    
    // Iterar sobre cada carácter hasta llegar al terminador nulo '\0'
    while (*str) {
        hash ^= (u32)((unsigned char)*str);
        hash *= FNV_PRIME;
        str++;
    }
    
    return hash;
}

static bool initialized = false;

static u16 maxAtlasSize = 0;

static T3DA_AtlasTexture** allAtlas;
static u16 atlasSize = 0;

const char *vertexShader = "\
#version 330 compatibility\n\
\n\
varying vec4 oColor;     // <- Usar varying asegura compatibilidad absoluta\n\
varying vec2 v_texCoord; \n\
\n\
void main()\n\
{\n\
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n\
    oColor = gl_Color;\n\
    v_texCoord = gl_MultiTexCoord0.xy;\n\
}\n\
";

const char* fragmentShader = "\
#version 330 compatibility\n\
\n\
uniform sampler2D u_texture;\n\
uniform vec4 u_tint;\n\
uniform float u_blend; // 0.0 = textura normal, 1.0 = tinte puro (estilo 3DS)\n\
\n\
varying vec2 v_texCoord;\n\
varying vec4 oColor;\n\
\n\
void main() {\n\
    // 1. Obtenemos el color original del pixel de la textura\n\
    vec4 texColor = texture2D(u_texture, v_texCoord);\n\
    \n\
    // 2. Mezclamos el color RGB de la textura con el RGB del tinte directamente\n\
    // Si u_blend es 1.0, el RGB pasa a ser el color exacto de u_tint (ej: blanco puro)\n\
    vec3 mixedRGB = mix(texColor.rgb, u_tint.rgb, u_blend);\n\
    \n\
    // 3. Reconstruimos el color combinando el RGB mezclado y conservando los Alfas\n\
    // texColor.a: mantiene la silueta/transparencia del sprite original\n\
    // u_tint.a: permite que el tinte en sí tenga opacidad independiente\n\
    // oColor.a: aplica el multiplicador de opacidad global 'b' enviado desde la CPU\n\
    gl_FragColor = vec4(mixedRGB, texColor.a * u_tint.a * oColor.a);\n\
}\n\
";

void t3da_init(u16 maxAtlas)
{
    if (initialized)
        return;

    allAtlas = NULL;

    initialized = true;

    sW = 0;
    sH = 0;
    // aW = 0;
    // aH = 0;

    maxAtlasSize = maxAtlas;

    allAtlas = malloc(sizeof(*allAtlas) * maxAtlasSize);
    atlasSize = 0;

#if defined(PLATFORM_PC)

    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader = SPOGL_CreateShader();

    if(!SPOGL_CompileShader(shader, vertexShader) || !SPOGL_CompileFragmentShader(shader, fragmentShader))
        SPOGL_Destroy(shader);
    
    SPOGL_LinkProgram(shader);

#elif defined(PLATFORM_3DS)

    romfsInit();

#endif
}

void t3da_begin_frame()
{
#if defined(PLATFORM_PC)

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

#endif
}

void t3da_end_frame()
{
    // SDL_GL_SwapWindow se hace fuera
}

#if defined(PLATFORM_PC)
SDL_Surface* CargarImagenDesdeMemoria(const void* buffer_datos, int tamano_bytes) {
    // 1. Crear el objeto RWops apuntando a la memoria
    SDL_RWops* rw = SDL_RWFromMem((void*)buffer_datos, tamano_bytes);
    if (!rw) {
        SDL_Log("Error al crear RWops: %s", SDL_GetError());
        return NULL;
    }

    // 2. Cargar la imagen desde el RWops
    // El '1' final libera automáticamente la estructura 'rw' al terminar
    SDL_Surface* superficie = IMG_Load_RW(rw, 1);
    if (!superficie) {
        SDL_Log("Error al cargar la imagen: %s", IMG_GetError());
        return NULL;
    }

    return superficie;
}

SDL_Surface* CargarImagenDesdePAK(const char* path) {
    // 1. Abrir el archivo dentro del paquete PAK
    PAK_FILE* f = PAKL_LoadFile(path);
    if (!f) return NULL;

    // 2. Calcular el tamaño del archivo usando tus funciones de posición
    PAKL_fseek(f, 0, SEEK_END);
    long tamano = PAKL_ftell(f);
    PAKL_rewind(f);

    if (tamano <= 0) {
        PAKL_CloseFile(f);
        return NULL;
    }

    // 3. Asignar memoria temporal para almacenar el archivo completo
    void* buffer = malloc(tamano);
    if (!buffer) {
        PAKL_CloseFile(f);
        return NULL;
    }

    // 4. Leer los bytes desde el PAK al búfer de memoria
    size_t leidos = PAKL_fread(buffer, 1, tamano, f);
    PAKL_CloseFile(f); // Ya no necesitamos el archivo abierto

    // 5. Pasar los datos RAM a tu función basada en SDL
    SDL_Surface* loaded = NULL;
    if (leidos == (size_t)tamano) {
        loaded = CargarImagenDesdeMemoria(buffer, tamano);
    }

    // 6. Liberar la memoria temporal del búfer
    free(buffer);

    return loaded;
}
#endif

T3DA_AtlasTexture *t3da_get_atlas(const char *path)
{
    u32 hash = calculate_hash(path);

    for(u16 i = 0; i < atlasSize; i++)
    {
        if(allAtlas[i]->hash == hash)
            return allAtlas[i];
    }

    if(atlasSize >= maxAtlasSize)
        return NULL;

    T3DA_AtlasTexture *atlas = malloc(sizeof(T3DA_AtlasTexture));

    if (!atlas)
        return NULL;

    atlas->path = strdup(path);

    Info info;

#if defined(PLATFORM_PC)

    char p[512];
    snprintf(p, sizeof(p), "%s.atlas", path);

    printf("Getting info.\n");
    info = getJsonInfo(p);
    printf("Info loaded.\n");

    printf("tiles: %d.\n", atlas->tiles);
    //printf("Offset Top: %d, Offset Left: %d, Offset Bottom: %d, Offset Right: %d.\n", info.offsetTop, info.offsetLeft, info.offsetBottom, info.offsetRight);
    //printf("tileX: %d, tileY: %d\n", (int)info.tileX, (int)info.tileY);

    if (!is_power_of_two(info.w) ||
        !is_power_of_two(info.h))
    {
        printf("[ERROR] Free info.\n");
        freeInfo(&info);
        printf("[ERROR] Free path.\n");
        free((void *)atlas->path);
        printf("[ERROR] Free atlas.\n");
        free(atlas);
        printf("[ERROR] Free all correctly.\n");
        return NULL;
    }

    SDL_Surface *surf =
        SDL_CreateRGBSurfaceWithFormat(
            0,
            info.w,
            info.h,
            32,
            SDL_PIXELFORMAT_ABGR8888);

    if (!surf)
    {
        freeInfo(&info);
        free((void *)atlas->path);
        free(atlas);
        return NULL;
    }

    for (size_t i = 0; i < info.imageCount; i++)
    {
        if (!info.images[i])
            continue;

        SDL_Surface *loaded = CargarImagenDesdePAK(info.images[i]->path);

        /*SDL_Surface *loaded =
            IMG_Load(info.images[i]->path);*/

        if (!loaded)
            continue;

        SDL_Surface *img =
            SDL_ConvertSurfaceFormat(
                loaded,
                SDL_PIXELFORMAT_ABGR8888,
                0);

        SDL_FreeSurface(loaded);

        if (!img)
            continue;

        SDL_Rect dst = {
            info.images[i]->x,
            info.images[i]->y,
            img->w,
            img->h};

        SDL_BlitSurface(img, NULL, surf, &dst);

        SDL_FreeSurface(img);
    }

    // Crear textura OpenGL
    glGenTextures(1, &atlas->sheet);

    glBindTexture(GL_TEXTURE_2D, atlas->sheet);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_NEAREST);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_NEAREST);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        surf->w,
        surf->h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        surf->pixels);

    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(surf);

#elif defined(PLATFORM_3DS)

    char p[512];
    snprintf(p, sizeof(p), "%s.t3x", path);

    FILE *f = fopen(getRomfsPath(p), "rb");

    if (!f)
    {
        free(atlas->path);
        free(atlas);
        return NULL;
    }

    TEX3DSHeader hdr;

    if (fread(&hdr, sizeof(hdr), 1, f) != 1)
    {
        fclose(f);
        return NULL;
    }

    if (memcmp(hdr.magic, "T3DS", 4) != 0)
    {
        fclose(f);
        return NULL;
    }

    info.w = hdr.width;
    info.h = hdr.height;
    info.tiles = hdr.tiles;

    if(f)
    {
        fseek(f, sizeof(TEX3DSHeader), SEEK_SET);

        setvbuf(f, NULL, _IOFBF, 64*1024);

        atlas->sheet = C2D_SpriteSheetLoadFromHandle(f);

        fclose(f);
    }

    if (!atlas->sheet) {
        printf("Failed loading spritesheet: %s\n", p);
        free(atlas->path);
        free(atlas);
        return NULL;
    }

    C2D_Image img =
        C2D_SpriteSheetGetImage(atlas->sheet, 0);

    C3D_Tex *tex = img.tex;
    atlas->tex = tex;
    
    C3D_TexSetFilter(tex, GPU_NEAREST, GPU_NEAREST);
    C3D_TexSetWrap(tex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);

    info.w = tex->width;
    info.h = tex->height;

    //printf("Suposed tileX: %d, and tileY: %d, let max size: %d.\nBut real max size: %d.\n", 
    //    info.tileX, info.tileY, info.tileX * info.tileX + info.tileY, tex->size);
#endif
    atlas->tiles = info.tiles;

    atlas->w = info.w;
    atlas->h = info.h;

    freeInfo(&info);

    atlas->hash = hash;

    // 1. Guardar en la posición actual
    allAtlas[atlasSize] = atlas;

    // 2. Verificar la posición actual donde acabas de escribir
    if (!allAtlas[atlasSize]) 
    {
        printf("[T3DA] Error: Writing error on memory.\n");
        free(atlas->path);
        free(atlas);
        return NULL;
    }

    // 3. Incrementar el tamaño solo si la asignación fue exitosa
    atlasSize++; 

    return atlas;
}


void t3da_get_atlas_size(T3DA_AtlasTexture *atlas, int *w, int *h)
{
    if(!atlas)
        return;
    if(w)
        *w = atlas->w;
    if(h)
        *h = atlas->h;
}
void t3da_get_atlas_tiles(T3DA_AtlasTexture *atlas, u8 *tile)
{
    if(!atlas)
        return;
    if(tile)
        *tile = atlas->tiles;
}


static void t3da_update_sprite_uvs(T3DA_DrawSprite *sprite)
{
    if (!sprite || !sprite->atlas)
        return;

#if defined(PLATFORM_PC)

    float tp = sprite->atlas->tilesValues[sprite->tile].top;
    float lf = sprite->atlas->tilesValues[sprite->tile].left;
    float rg = sprite->atlas->tilesValues[sprite->tile].right;
    float bt = sprite->atlas->tilesValues[sprite->tile].bottom;

    float tileW = rg - lf;
    float tileH = bt - tp;

    // Corrección de coordenadas UV
    sprite->u1 = lf; // Esquina izquierda (X1)
    sprite->v1 = tp; // Esquina superior (Y1)

    sprite->u2 = rg; // Esquina derecha (X2)
    sprite->v2 = bt; // Esquina inferior (Y2)

#elif defined(PLATFORM_3DS)

    if(sprite->changedTiles)
    {
        sprite->image =
            C2D_SpriteSheetGetImage(sprite->atlas->sheet, sprite->tile);

        sprite->subtex = fix_UV(&sprite->image);

        sprite->changedTiles = false;
    }

#endif
}

#if defined(PLATFORM_3DS)

static void t3da_update_params(T3DA_DrawSprite *sp)
{
    // sp->params.pos.x = sp->x + sW * aW;
    // sp->params.pos.y = sp->y + sH * aH;

    // sp->params.pos.w = sp->w;
    // sp->params.pos.h = sp->h;

    sp->scaleX = sp->w / sp->ix;
    sp->scaleY = sp->h / sp->iy;
}

#endif

T3DA_DrawSprite *t3da_get_sprite_from_atlas(
    T3DA_AtlasTexture *atlas,
    u8 tile,
    float x,
    float y,
    float depth,
    float r,
    float w,
    float h,
    float alignX,
    float alignY)
{
    if (!atlas)
        return NULL;

    float tp = atlas->tilesValues[tile].top;
    float lf = atlas->tilesValues[tile].left;
    float rg = atlas->tilesValues[tile].right;
    float bt = atlas->tilesValues[tile].bottom;

    float tileW = (rg - lf)*atlas->w;
    float tileH = (bt - tp)*atlas->h;

    if (w == 0)
        w = tileW;

    if (h == 0)
        h = tileH;

    T3DA_DrawSprite *sp =
        malloc(sizeof(T3DA_DrawSprite));

    if (!sp)
        return NULL;

    memset(sp, 0, sizeof(T3DA_DrawSprite));
    
    sp->tile = tile;

    sp->atlas = atlas;

    sp->ix = tileW;
    sp->iy = tileH;

    sp->w = w;
    sp->h = h;

    sp->scaleX = w/tileW;
    sp->scaleY = h/tileH;

    sp->x = x;
    sp->y = y;

    sp->depth = depth;

    sp->alignX = alignX <= 0.0f ? 0.0f : (alignX >= 1.0f ? 1.0f : alignX);
    sp->alignY = alignY <= 0.0f ? 0.0f : (alignY >= 1.0f ? 1.0f : alignY);

#if defined(PLATFORM_PC)
    sp->rotation = r;

    sp->img = atlas->sheet;

#elif defined(PLATFORM_3DS)
    sp->rotation = C3D_AngleFromDegrees(r);

    t3da_update_params(sp);

    sp->changedTiles = true;
    
#endif
    t3da_update_sprite_uvs(sp);

    return sp;
}

// Set functions for sprite

void t3da_set_sprite(
    T3DA_DrawSprite *sprite,
    u8 tile)
{
    if (!sprite)
        return;

    sprite->tile = tile;

#if defined(PLATFORM_3DS)
    sprite->changedTiles = true;
#endif

    t3da_update_sprite_uvs(sprite);
}

// ======================================================
// POSITION
// ======================================================
void t3da_set_sprite_position_x(T3DA_DrawSprite *sprite, float x)
{
    if (!sprite) return;

    sprite->x = x;
#if defined(PLATFORM_3DS)
    t3da_update_params(sprite);
#endif
}

void t3da_set_sprite_position_y(T3DA_DrawSprite *sprite, float y)
{
    if (!sprite) return;

    sprite->y = y;
#if defined(PLATFORM_3DS)
    t3da_update_params(sprite);
#endif
}
void t3da_set_sprite_position(T3DA_DrawSprite *sprite, float x, float y)
{
    t3da_set_sprite_position_x(sprite, x);
    t3da_set_sprite_position_y(sprite, y);
}

void t3da_set_sprite_depth(T3DA_DrawSprite *sprite, float depth)
{
    if(!sprite)
        return;
#if defined(PLATFORM_PC)
    sprite->depth = depth;
#elif defined(PLATFORM_3DS)
    // sprite->params.depth = depth;
    sprite->depth = depth;
#endif
}
void t3da_set_sprite_rotation(T3DA_DrawSprite *sprite, float r)
{
    if(!sprite)
        return;
#if defined(PLATFORM_PC)
    sprite->rotation = r;
#elif defined(PLATFORM_3DS)
    // sprite->params.angle = C3D_AngleFromDegrees(r);
    sprite->rotation = C3D_AngleFromDegrees(r);
#endif
}

void t3da_set_sprite_width(T3DA_DrawSprite *sprite, float w)
{
    if(!sprite)
        return;
    sprite->w = w;
    sprite->scaleX = w/sprite->ix;
    t3da_update_sprite_uvs(sprite);
}
void t3da_set_sprite_height(T3DA_DrawSprite *sprite, float h)
{
    if(!sprite)
        return;
    sprite->h = h;
    sprite->scaleY = h/sprite->iy;
    t3da_update_sprite_uvs(sprite);
}
void t3da_set_sprite_size(T3DA_DrawSprite *sprite, float w, float h)
{
    t3da_set_sprite_width(sprite, w);
    t3da_set_sprite_height(sprite, h);
}

// ======================================================
// SCALE (FIXED)
// ======================================================
void t3da_set_sprite_scale_x(T3DA_DrawSprite *sprite, float scale)
{
    if (!sprite) return;
    if (sprite->scaleX == scale) return;
    sprite->scaleX = scale;
    sprite->w = sprite->ix * scale;
    t3da_update_sprite_uvs(sprite);
}

void t3da_set_sprite_scale_y(T3DA_DrawSprite *sprite, float scale)
{
    if (!sprite) return;
    if (sprite->scaleY == scale) return;
    sprite->scaleY = scale;
    sprite->h = sprite->iy * scale;
    t3da_update_sprite_uvs(sprite);
}

void t3da_set_sprite_scale(T3DA_DrawSprite *sprite, float sx, float sy)
{
    t3da_set_sprite_scale_x(sprite, sx);
    t3da_set_sprite_scale_y(sprite, sy);
}

void t3da_set_sprite_align_x(T3DA_DrawSprite *sprite, float alignX)
{
    if(!sprite)
        return;
    sprite->alignX = alignX <= 0.0f ? 0.0f : (alignX >= 1.0f ? 1.0f : alignX);
}
void t3da_set_sprite_align_y(T3DA_DrawSprite *sprite, float alignY)
{
    if(!sprite)
        return;
    sprite->alignY = alignY <= 0.0f ? 0.0f : (alignY >= 1.0f ? 1.0f : alignY);
}
void t3da_set_sprite_align(T3DA_DrawSprite *sprite, float alignX, float alignY)
{
    t3da_set_sprite_align_x(sprite, alignX);
    t3da_set_sprite_align_y(sprite, alignY);
}

void t3da_get_sprite_values(T3DA_DrawSprite *sprite, u8 *tile, float *x, float *y, float *depth, float *r, float *w, float *h, float *alignX, float *alignY)
{
    if(!sprite)
        return;

        printf(
    "tile=(%d) pos=(%f,%f) size=(%f,%f) align=(%f,%f)\n",
    sprite->tile,
    sprite->x,
    sprite->y,
    sprite->w,
    sprite->h,
    sprite->alignX,
    sprite->alignY
);

    if(tile)
        *tile = sprite->tile;

    if(x)
        *x = sprite->x;
    if(y)
        *y = sprite->y;

    if(depth)
        *depth = sprite->depth;

    if(w)
        *w = sprite->w;
    if(h)
        *h = sprite->h;

    if(alignX)
        *alignX = sprite->alignX;
    if(alignY)
        *alignY = sprite->alignY;

#if defined(PLATFORM_PC)
    if(r)
        *r = sprite->rotation;
#elif defined(PLATFORM_3DS)
    if(r)
        *r = AngleToDegrees(sprite->rotation);
#endif
}


void t3da_draw_sprite(T3DA_DrawSprite *sprite, float sAlignX, float sAlignY, Color tint, float blend)
{
    if (!sprite)
        return;

    if(!isValidScreen())
        return;

    sAlignX = clampf(sAlignX, 0.f, 1.f);
    sAlignY = clampf(sAlignY, 0.f, 1.f);
    float b = clampf(blend, 0.f, 1.f);

#if defined(PLATFORM_PC)

    int w = sprite->w * windowScale;
    int h = sprite->h * windowScale;
    int x = sprite->x * windowScale;
    int y = sprite->y * windowScale;

    // 2. Sumamos el punto de inicio de la pantalla actual (TOP o BOTTOM)
    x += (currScreen == TOP ? topInitialPointX : bottomInitialPointX);
    y += (currScreen == TOP ? topInitialPointY : bottomInitialPointY);

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
    SPOGL_Use(shader);

    glBindTexture(GL_TEXTURE_2D, sprite->img);

    glColor4ub(
        tint.r,
        tint.g,
        tint.b,
        tint.a);

    float ax = w * sprite->alignX - sW * windowScale * sAlignX;
    float ay = h * sprite->alignY - sH * windowScale * sAlignY;

    float x1 = -ax;
    float y1 = -ay;

    float x2 = w - ax;
    float y2 = h - ay;

    glPushMatrix();

    glTranslatef(
        x,
        y,
        sprite->depth);

    glRotatef(sprite->rotation, 0.f, 0.f, 1.f);
    
    GLuint openGLProgramID = shader->gProgram;

    // 3. Enviamos los uniforms INMEDIATAMENTE después utilizando el ID correcto
    GLint tintLoc  = glGetUniformLocation(openGLProgramID, "u_tint");
    GLint blendLoc = glGetUniformLocation(openGLProgramID, "u_blend");

    if (tintLoc != -1) {
        glUniform4f(tintLoc, tint.r / 255.f, tint.g / 255.f, tint.b / 255.f, tint.a / 255.f);
    }
    
    if (blendLoc != -1) {
        // En lugar de 'b', aquí debes pasar el factor real de tinte (de 0.0 a 1.0)
        // Si tu struct sprite tiene una variable para ello, usa: sprite->blendFactor
        glUniform1f(blendLoc, clampf(b, 0.f, 1.f)); 
    }

    glBegin(GL_QUADS);
        // Ya no necesitas usar glColor4ub aquí, el shader se encarga de todo
        glTexCoord2f(sprite->u2, sprite->v2); glVertex2f(x2, y2);
        glTexCoord2f(sprite->u2, sprite->v1); glVertex2f(x2, y1);
        glTexCoord2f(sprite->u1, sprite->v1); glVertex2f(x1, y1);
        glTexCoord2f(sprite->u1, sprite->v2); glVertex2f(x1, y2);
    glEnd();

    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, 0);

#elif defined(PLATFORM_3DS)

    for(int i = 0; i < 4; i++) {
        sprite->tint.corners[i].color = Color_ToUInt32_Default(tint);
        sprite->tint.corners[i].blend = b;
    }

    t3da_update_params(sprite);

    //C2D_DrawImage(sprite->image, &sprite->params, &sprite->tint);
    C2D_DrawImageAtRotated(sprite->image, (sprite->x + sW * sAlignX - sprite->w/2.0f + sprite->w * sprite->alignX), (sprite->y + sH * sAlignY - sprite->h/2.0f + sprite->h * sprite->alignY), sprite->depth, sprite->rotation, &sprite->tint, sprite->scaleX, sprite->scaleY);

#endif
}

void t3da_set_screen_size(u16 w, u16 h)
{
    sW = w;
    sH = h;
}
/*
void t3da_set_screen_draw_align(float x, float y)
{
    aW = x <= 0.0f ? 0.0f : (x >= 1.0f ? 1.0f : x);
    aH = y <= 0.0f ? 0.0f : (y >= 1.0f ? 1.0f : y);
}
*/

void t3da_free_sprite(T3DA_DrawSprite *sprite)
{
    if (!sprite)
        return;
        
#if defined(PLATFORM_3DS)
    if(sprite->subtex)
        free(sprite->subtex);
#endif

    free(sprite);
}

void t3da_free_atlas(T3DA_AtlasTexture *atlas)
{
    if (!atlas)
        return;

#if defined(PLATFORM_PC)

    if (atlas->sheet)
        glDeleteTextures(1, &atlas->sheet);

#elif defined(PLATFORM_3DS)

    if (atlas->sheet)
        C2D_SpriteSheetFree(atlas->sheet);

#endif

    free((void *)atlas->path);
    free(atlas);
}

void t3da_exit()
{
    if (!initialized)
        return;

    initialized = false;

#if defined(PLATFORM_PC)

    SPOGL_Destroy(shader);
    IMG_Quit();

#elif defined(PLATFORM_3DS)

    romfsExit();

#endif
}