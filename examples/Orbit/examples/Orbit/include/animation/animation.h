#pragma once

#include <stdbool.h>
#include <textures/textures.h>

typedef unsigned int uint;

typedef struct A3D_Animation2D A3D_Animation2D;

#ifdef __cplusplus
extern "C"
{
#endif

void a3d_init();

double a3d_current_time();

void a3d_begin_frame(double deltaTime);

void a3d_end_frame();

A3D_Animation2D *
a3d_create_animation_2d(bool loop);

void a3d_free_animation_2d(
    A3D_Animation2D *anim);

void a3d_set_animation_2d_loop(
    A3D_Animation2D *anim,
    bool loop);

bool a3d_is_2d_animation_loop(
    A3D_Animation2D *anim);

bool a3d_is_2d_animation_playing(
    A3D_Animation2D *anim);

bool a3d_play_2d_anim(
    A3D_Animation2D *anim);

bool a3d_pause_2d_anim(
    A3D_Animation2D *anim);

bool a3d_stop_2d_anim(
    A3D_Animation2D *anim);

bool a3d_set_2d_anim_keyframe(
    A3D_Animation2D *anim,
    double seconds, 
    T3DA_DrawSprite *image);

T3DA_DrawSprite *
a3d_get_2d_anim_seconds_sprite(
    A3D_Animation2D *anim,
    double seconds);

T3DA_DrawSprite *
a3d_get_2d_anim_current_frame_sprite(
    A3D_Animation2D *anim);

void a3d_exit();

#ifdef __cplusplus
}
#endif