#include <animation/animation.h>
#include <textures/textures.h>

#include <sys/types.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <math.h>

typedef struct A3D_KeyFrame2D
{
    double seconds;
    T3DA_DrawSprite *img;
} A3D_KeyFrame2D;

typedef struct A3D_Animation2D
{
    bool looping;

    double startingTime;

    double pausedTime;

    bool paused;

    uint keyframes;

    double maxSeconds;

    A3D_KeyFrame2D **frames;

} A3D_Animation2D;

static bool initialized = false;
static double currentTime = 0.0;

static A3D_Animation2D **animations = NULL;
static size_t animationsCount = 0;

/* -------------------- CORE -------------------- */

void a3d_init()
{
    if (initialized)
        return;

    initialized = true;
    currentTime = 0.0;
    animations = NULL;
    animationsCount = 0;
}

double a3d_current_time()
{
    return currentTime;
}

void a3d_begin_frame(double deltaTime)
{
    currentTime += deltaTime;
}

void a3d_end_frame()
{
    for (size_t i = 0; i < animationsCount; i++)
    {
        A3D_Animation2D *anim = animations[i];

        if (!anim || anim->paused)
            continue;

        if (!anim->looping)
        {
            double t = currentTime - anim->startingTime;

            if (anim->maxSeconds > 0.0 && t >= anim->maxSeconds)
            {
                anim->paused = true;
                anim->pausedTime = anim->maxSeconds;
            }
        }
    }
}

/* -------------------- ANIMATION -------------------- */

A3D_Animation2D *a3d_create_animation_2d(bool loop)
{
    A3D_Animation2D *anim = calloc(1, sizeof(A3D_Animation2D));
    if (!anim)
        return NULL;

    anim->looping = loop;
    anim->paused = true;
    anim->startingTime = currentTime;

    A3D_Animation2D **tmp =
        realloc(animations, (animationsCount + 1) * sizeof(A3D_Animation2D *));

    if (!tmp)
    {
        free(anim);
        return NULL;
    }

    animations = tmp;
    animations[animationsCount++] = anim;

    return anim;
}

void a3d_set_animation_2d_loop(A3D_Animation2D *anim, bool loop)
{
    if (anim)
        anim->looping = loop;
}

bool a3d_is_2d_animation_loop(A3D_Animation2D *anim)
{
    return anim ? anim->looping : false;
}

/* -------------------- LIFECYCLE -------------------- */

void a3d_free_animation_2d(A3D_Animation2D *anim)
{
    if (!anim)
        return;

    for (size_t i = 0; i < animationsCount; i++)
    {
        if (animations[i] == anim)
        {
            animations[i] = animations[animationsCount - 1];
            animationsCount--;
            break;
        }
    }

    for (uint i = 0; i < anim->keyframes; i++)
        free(anim->frames[i]);

    free(anim->frames);
    free(anim);
}

/* -------------------- CONTROL -------------------- */

bool a3d_play_2d_anim(A3D_Animation2D *anim)
{
    if (!anim)
        return false;

    if (anim->paused)
    {
        // Resume from where it was paused
        anim->startingTime = currentTime - anim->pausedTime;
    }

    anim->paused = false;

    return true;
}

bool a3d_pause_2d_anim(A3D_Animation2D *anim)
{
    if (!anim)
        return false;

    anim->paused = true;

    double t = currentTime - anim->startingTime;

    if (anim->maxSeconds > 0.0)
        t = fmod(t, anim->maxSeconds);

    anim->pausedTime = t;

    return true;
}

bool a3d_stop_2d_anim(A3D_Animation2D *anim)
{
    if (!anim)
        return false;

    anim->paused = true;
    anim->pausedTime = 0.0;
    anim->startingTime = currentTime;

    return true;
}

bool a3d_is_2d_animation_playing(A3D_Animation2D *anim)
{
    if (!anim)
        return false;

    return !anim->paused;
}

/* -------------------- KEYFRAMES -------------------- */

static void a3d_free_keyframe_2d(A3D_KeyFrame2D *frame)
{
    if (frame)
        free(frame);
}

/* -------------------- INSERT KEYFRAME -------------------- */

bool a3d_set_2d_anim_keyframe(A3D_Animation2D *anim, double seconds, T3DA_DrawSprite *image)
{
    if (!anim || !image)
        return false;

    A3D_KeyFrame2D *keyframe = calloc(1, sizeof(A3D_KeyFrame2D));
    if (!keyframe)
        return false;

    keyframe->seconds = seconds;
    keyframe->img = image;

    uint insertIndex = anim->keyframes;

    for (uint i = 0; i < anim->keyframes; i++)
    {
        if (anim->frames[i]->seconds > seconds)
        {
            insertIndex = i;
            break;
        }
    }

    A3D_KeyFrame2D **tmp =
        realloc(anim->frames, (anim->keyframes + 1) * sizeof(A3D_KeyFrame2D *));

    if (!tmp)
    {
        free(keyframe);
        return false;
    }

    anim->frames = tmp;

    for (uint i = anim->keyframes; i > insertIndex; i--)
        anim->frames[i] = anim->frames[i - 1];

    anim->frames[insertIndex] = keyframe;
    anim->keyframes++;

    if (seconds > anim->maxSeconds)
        anim->maxSeconds = seconds;

    return true;
}

/* -------------------- GET FRAME BY TIME -------------------- */

T3DA_DrawSprite *
a3d_get_2d_anim_seconds_sprite(A3D_Animation2D *anim, double seconds)
{
    if (!anim || anim->keyframes == 0)
        return NULL;

    double t = seconds;

    if (anim->looping && anim->maxSeconds > 0.0)
        t = fmod(t, anim->maxSeconds);

    T3DA_DrawSprite *last = NULL;

    for (uint i = 0; i < anim->keyframes; i++)
    {
        A3D_KeyFrame2D *kf = anim->frames[i];
        if (!kf)
            continue;

        if (kf->seconds == t)
            return kf->img;

        if (kf->seconds < t)
            last = kf->img;
        else
            break;
    }

    // if there is no previous valid frame (case t < first keyframe)
    if (!last)
        return anim->frames[anim->keyframes - 1]->img;

    return last;
}

T3DA_DrawSprite *
a3d_get_2d_anim_current_frame_sprite(A3D_Animation2D *anim)
{
    if (!anim)
        return NULL;

    if (anim->paused)
        return a3d_get_2d_anim_seconds_sprite(anim, anim->pausedTime);

    double t = currentTime - anim->startingTime;

    return a3d_get_2d_anim_seconds_sprite(anim, t);
}

/* -------------------- EXIT -------------------- */

void a3d_exit()
{
    if (!initialized)
        return;

    initialized = false;

    while (animationsCount > 0)
        a3d_free_animation_2d(animations[0]);

    free(animations);

    animations = NULL;
    animationsCount = 0;
    currentTime = 0.0;
}