#pragma once

#if defined(PLATFORM_PC)

#include <SDL_mixer.h>

#elif defined(PLATFORM_3DS)

#include <3ds.h>
#include "opusfile.h"

#endif

#include <stdbool.h>


/* ============================================================
 * CONFIG
 * ============================================================ */

#define OPUS_CHUNK_FRAMES 16384

#define NDSP_QUEUED_BUFFERS 6

#define CHANNELS 24

#define MAX_VOLUME 100

#define AUDIO_SAMPLE_RATE 48000


/* ============================================================
 * SOUND
 * ============================================================ */

typedef struct S3D_Sound
{
    int type;


    /*
     * Estado:
     *
     * playing = true
     *     El sonido tiene una reproducción activa.
     *
     * paused = false
     *     Está sonando.
     *
     * paused = true
     *     Está pausado, pero NO está detenido.
     *
     * playing = false
     *     No hay reproducción activa.
     */
    bool playing;

    bool paused;


    /*
     * Canal de audio.
     *
     * -1 = ningún canal.
     */
    int channel;


#if defined(PLATFORM_PC)

    Mix_Chunk *chunk;


#elif defined(PLATFORM_3DS)

    OggOpusFile *opus;


    /*
     * Buffers PCM lineales.
     */
    short *pcmBuffer[NDSP_QUEUED_BUFFERS];


    /*
     * WaveBuf usados por NDSP.
     */
    ndspWaveBuf waveBufs[NDSP_QUEUED_BUFFERS];


    /*
     * Buffer utilizado actualmente para
     * el streaming.
     */
    int currentBuffer;


    /*
     * Número de canales PCM.
     */
    int pcmChannels;


    /*
     * Repeticiones:
     *
     * 0 = infinito
     * 1 = una reproducción
     * 2 = dos reproducciones
     * etc.
     */
    int repeats;


    /*
     * true cuando el decoder ya ha llegado
     * al final definitivo.
     *
     * Los WaveBuf que ya estén en NDSP
     * deben terminar normalmente.
     */
    bool finished;

#endif

} S3D_Sound;


/* ============================================================
 * 3DS OPUS
 * ============================================================ */

#if defined(PLATFORM_3DS)

#ifdef __cplusplus
extern "C"
{
#endif


bool CargarAudioOpus(
    const char *ruta,
    S3D_Sound *sound
);


int fill_opus_chunk(
    S3D_Sound *sound,
    int bufIndex
);


#ifdef __cplusplus
}
#endif

#endif