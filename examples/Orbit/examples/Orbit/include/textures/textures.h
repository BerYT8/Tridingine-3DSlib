#pragma once

/**
 * @file t3da.h
 * @brief Main public API for the T3DA rendering system.
 *
 * Provides atlas management, sprite creation, sprite manipulation,
 * rendering, and frame lifecycle functions for Nintendo 3DS.
 */

#include "t3da_types.h"
#include <color.h>

#include <ints_defs.h>

#define DEFAULT_MAX_ATLAS 1024
#define DEFAULT_MAX_ATLAS_X2 2048

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initializes the T3DA rendering system.
     *
     * Must be called before using any other T3DA function.
     * @param maxAtlas Max atlas size can be used both.
     */
    void t3da_init(u16 maxAtlas);

    /**
     * @brief Begins a new rendering frame.
     *
     * Call before drawing any sprites.
     */
    void t3da_begin_frame();

    /**
     * @brief Ends the current rendering frame.
     *
     * Presents the rendered frame to the screen.
     */
    void t3da_end_frame();

    /**
     * @brief Loads a texture atlas from disk.
     *
     * The function expects a `.t3x` texture atlas.
     *
     * @param path Path to the atlas without extension.
     * @return Pointer to the created atlas, or NULL on failure.
     */
    T3DA_AtlasTexture *t3da_get_atlas(const char *path);

    /**
     * @brief Creates a drawable sprite from an atlas tile.
     *
     * @param atlas Source atlas.
     * @param tile Tile index.
     * @param x World X position.
     * @param y World Y position.
     * @param depth Sprite depth.
     * @param r Rotation in radians.
     * @param w Sprite width.
     * @param h Sprite height.
     * @param alignX Horizontal alignment.
     * @param alignY Vertical alignment.
     *
     * @return Pointer to the created sprite.
     */
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
        float alignY);

    /**
     * @brief Retrieves all sprite values.
     *
     * Any output pointer may be NULL.
     *
     * @param sprite Target sprite.
     * @param tile Output tile index.
     * @param x Output X position.
     * @param y Output Y position.
     * @param depth Output depth.
     * @param r Output rotation.
     * @param w Output width.
     * @param h Output height.
     * @param alignX Output horizontal alignment.
     * @param alignY Output vertical alignment.
     */
    void t3da_get_sprite_values(
        T3DA_DrawSprite *sprite,
        u8 *tile,
        float *x,
        float *y,
        float *depth,
        float *r,
        float *w,
        float *h,
        float *alignX,
        float *alignY);

    /**
     * @brief Changes the atlas tile used by a sprite.
     *
     * @param sprite Target sprite.
     * @param tileX New tile X coordinate.
     * @param tileY New tile Y coordinate.
     */
    void t3da_set_sprite(
        T3DA_DrawSprite *sprite,
        u8 tile);

    /**
     * @brief Sets the sprite X position.
     *
     * @param sprite Target sprite.
     * @param x New X position.
     */
    void t3da_set_sprite_position_x(T3DA_DrawSprite *sprite, float x);

    /**
     * @brief Sets the sprite Y position.
     *
     * @param sprite Target sprite.
     * @param y New Y position.
     */
    void t3da_set_sprite_position_y(T3DA_DrawSprite *sprite, float y);

    /**
     * @brief Sets the sprite position.
     *
     * @param sprite Target sprite.
     * @param x New X position.
     * @param y New Y position.
     */
    void t3da_set_sprite_position(T3DA_DrawSprite *sprite, float x, float y);

    /**
     * @brief Sets the sprite depth.
     *
     * @param sprite Target sprite.
     * @param depth New depth value.
     */
    void t3da_set_sprite_depth(T3DA_DrawSprite *sprite, float depth);

    /**
     * @brief Sets the sprite rotation.
     *
     * @param sprite Target sprite.
     * @param r Rotation in radians.
     */
    void t3da_set_sprite_rotation(T3DA_DrawSprite *sprite, float r);

    /**
     * @brief Sets the sprite width.
     *
     * @param sprite Target sprite.
     * @param w New width.
     */
    void t3da_set_sprite_width(T3DA_DrawSprite *sprite, float w);

    /**
     * @brief Sets the sprite height.
     *
     * @param sprite Target sprite.
     * @param h New height.
     */
    void t3da_set_sprite_height(T3DA_DrawSprite *sprite, float h);

    /**
     * @brief Sets the sprite size.
     *
     * @param sprite Target sprite.
     * @param w New width.
     * @param h New height.
     */
    void t3da_set_sprite_size(T3DA_DrawSprite *sprite, float w, float h);

    void t3da_set_sprite_scale_x(T3DA_DrawSprite *sprite, float scale);
    void t3da_set_sprite_scale_y(T3DA_DrawSprite *sprite, float scale);
    void t3da_set_sprite_scale(T3DA_DrawSprite *sprite, float scaleX, float scaleY);

    /**
     * @brief Sets the sprite horizontal alignment.
     *
     * @param sprite Target sprite.
     * @param alignX Horizontal alignment.
     */
    void t3da_set_sprite_align_x(T3DA_DrawSprite *sprite, float alignX);

    /**
     * @brief Sets the sprite vertical alignment.
     *
     * @param sprite Target sprite.
     * @param alignY Vertical alignment.
     */
    void t3da_set_sprite_align_y(T3DA_DrawSprite *sprite, float alignY);

    /**
     * @brief Sets the sprite alignment.
     *
     * @param sprite Target sprite.
     * @param alignX Horizontal alignment.
     * @param alignY Vertical alignment.
     */
    void t3da_set_sprite_align(
        T3DA_DrawSprite *sprite,
        float alignX,
        float alignY);

    /**
     * @brief Retrieves the atlas texture size.
     *
     * @param atlas Target atlas.
     * @param w Output width.
     * @param h Output height.
     */
    void t3da_get_atlas_size(
        T3DA_AtlasTexture *atlas,
        int *w,
        int *h);

    /**
     * @brief Retrieves atlas tile counts.
     *
     * @param atlas Target atlas.
     * @param tileX Output horizontal tile count.
     * @param tileY Output vertical tile count.
     */
    void t3da_get_atlas_tiles(
        T3DA_AtlasTexture *atlas,
        u8 *tile);

    /**
     * @brief Frees an atlas and its resources.
     *
     * @param atlas Atlas to free.
     */
    void t3da_free_atlas(T3DA_AtlasTexture *atlas);

    /**
     * @brief Frees a sprite.
     *
     * @param sprite Sprite to free.
     */
    void t3da_free_sprite(T3DA_DrawSprite *sprite);

    /**
     * @brief Draws a sprite.
     *
     * @param sprite Sprite to draw.
     * @param sAlignX Screen x alignment.
     * @param sprite Screen y alignment.
     * @param tint Tint color.
     * @param blend Blend factor.
     */
    void t3da_draw_sprite(
        T3DA_DrawSprite *sprite,
        float sAlignX,
        float sAlignY,
        Color tint,
        float blend);

    /**
     * @brief Shuts down the T3DA rendering system.
     *
     * Frees all internal resources.
     */
    void t3da_exit();

#ifdef __cplusplus
}
#endif