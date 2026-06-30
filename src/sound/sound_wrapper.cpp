#if defined(PLATFORM_3DS)
#include "soundLoad.h"
#include <cstdio>
#include <3ds.h>
#include <cstring>

#include "opusfile.h"

bool CargarAudioOpus(const char *ruta, S3D_Sound *sound)
{
    int error;
    OggOpusFile *opus = op_open_file(ruta, &error);

    if (!opus)
    {
        printf("[OPUS] Error abrir: %d\n", error);
        return false;
    }

    const OpusHead *head = op_head(opus, 0);

    sound->opus = opus;
    sound->pcmChannels = head->channel_count;

    sound->currentBuffer = 0;
    sound->repeats = 0;
    sound->finished = false;

    size_t size = OPUS_CHUNK_FRAMES * sound->pcmChannels * sizeof(short);

    sound->pcmBuffer[0] = (short*)linearAlloc(size);
    sound->pcmBuffer[1] = (short*)linearAlloc(size);

    if (!sound->pcmBuffer[0] || !sound->pcmBuffer[1])
    {
        printf("[OPUS] No RAM\n");
        op_free(opus);
        return false;
    }

    memset(sound->waveBufs, 0, sizeof(sound->waveBufs));

    printf("[OPUS] OK channels=%d\n", sound->pcmChannels);
    return true;
}

int fill_opus_chunk(S3D_Sound *s, int bufIndex)
{
    if (!s || !s->opus) return 0;

    short *dst = s->pcmBuffer[bufIndex];

    int frames = op_read(
        s->opus,
        dst,
        OPUS_CHUNK_FRAMES,
        NULL
    );

    if (frames <= 0)
    {
        if (s->repeats != 0)
        {
            if (s->repeats > 0)
                s->repeats--;

            op_pcm_seek(s->opus, 0);
            return fill_opus_chunk(s, bufIndex);
        }

        s->finished = true;
        return 0;
    }

    DSP_FlushDataCache(dst, frames * s->pcmChannels * sizeof(short));
    return frames;
}

#endif