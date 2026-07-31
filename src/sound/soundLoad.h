
#if defined(PLATFORM_PC)
#include <SDL_mixer.h>
#elif defined(PLATFORM_3DS)
#include <3ds.h>
#include "opusfile.h"
#include <string.h>
#endif
#include <stdbool.h>

#define OPUS_CHUNK_FRAMES 4096
#define NDSP_QUEUED_BUFFERS 3

typedef struct S3D_Sound
{
    int type;
    bool playing;
    int channel;

#if defined(PLATFORM_PC)
    Mix_Chunk *chunk;
#endif

#if defined(PLATFORM_3DS)
    OggOpusFile *opus;

    short *pcmBuffer[NDSP_QUEUED_BUFFERS];
    int currentBuffer;

    ndspWaveBuf waveBufs[NDSP_QUEUED_BUFFERS];   // 🔥 IMPORTANTE: 2 waveBuf reales

    int pcmChannels;

    int repeats;
    bool finished;
#endif
} S3D_Sound;

#if defined(PLATFORM_3DS)
#pragma once

#include <sound/sound.h>

#ifdef __cplusplus
extern "C"
{
#endif

bool CargarAudioOpus(const char *ruta, S3D_Sound *sound);
int fill_opus_chunk(S3D_Sound *s, int bufIndex);

#ifdef __cplusplus
}
#endif

#endif