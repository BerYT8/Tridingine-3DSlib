#include <sound/sound.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h>

#define AUDIO_SAMPLE_RATE 48000

#if defined(PLATFORM_PC)
#include <SDL.h>

#elif defined(PLATFORM_3DS)
#include <3ds.h>
#endif
#include "soundLoad.h"

#include <stdbool.h>
#include <pak_loader/pak_loader.h>

#define MAX_VOLUME 100

static bool initialized = false;

int MasterVolume = MAX_VOLUME;
int SoundVolume = MAX_VOLUME;
int MusicVolume = MAX_VOLUME;
int VoiceVolume = MAX_VOLUME;
int AmbiVolume = MAX_VOLUME;

int clamp_volume(int v)
{
    if (v < 0)
        return 0;
    if (v > MAX_VOLUME)
        return MAX_VOLUME;
    return v;
}

void s3d_set_master_volume(int volume)
{
    MasterVolume = clamp_volume(volume);
}
int s3d_get_master_volume()
{
    return MasterVolume;
}

void s3d_set_sound_volume(int volume)
{
    SoundVolume = clamp_volume(volume);
}
int s3d_get_sound_volume()
{
    return SoundVolume;
}

void s3d_set_music_volume(int volume)
{
    MusicVolume = clamp_volume(volume);
}
int s3d_get_music_volume()
{
    return MusicVolume;
}

void s3d_set_voice_volume(int volume)
{
    VoiceVolume = clamp_volume(volume);
}
int s3d_get_voice_volume()
{
    return VoiceVolume;
}

void s3d_set_ambi_volume(int volume)
{
    AmbiVolume = clamp_volume(volume);
}
int s3d_get_ambi_volume()
{
    return AmbiVolume;
}

#define CHANNELS 24

bool used_channels[CHANNELS];

S3D_Sound *sound_channels[CHANNELS];

int channel_in_use(int channel)
{
    if (channel < 0 || channel >= CHANNELS)
        return -1;
#if defined(PLATFORM_3DS)
    return ndspChnIsPlaying(channel) ? 1 : used_channels[channel] ? 1
                                                                  : 0;
#elif defined(PLATFORM_PC)
    return Mix_Playing(channel) == 1 ? 1 : used_channels[channel] ? 1
                                                                  : 0;
#endif
}
int get_next_use_channel(int type)
{
    int startingIndex = 0;
    int endingIndex = CHANNELS - 1;

    switch (type)
    {
    case MUSIC_TYPE:
        endingIndex = 1;
        break;
    case SOUND_TYPE:
        startingIndex = 6;
        break;
    case VOICE_TYPE:
        startingIndex = 4;
        endingIndex = 5;
        break;
    case AMBI_TYPE:
        startingIndex = 2;
        endingIndex = 3;
        break;
    default:
        return -2;
    }

    for (int i = startingIndex; i <= endingIndex; i++)
    {
        if (channel_in_use(i) == 0)
        {
            return i;
        }
    }

    return -1;
}

int get_type_volume(int type)
{
    switch (type)
    {
    case SOUND_TYPE:
        return SoundVolume;
    case MUSIC_TYPE:
        return MusicVolume;
    case VOICE_TYPE:
        return VoiceVolume;
    case AMBI_TYPE:
        return AmbiVolume;
    }
    return 0;
}

int play_any(S3D_Sound *s, int volume, int channel, int repeats)
{
    if (!s) return S3D_PLAY_ERR_NO_SOUND;

    int ch = (channel <= -1) ? get_next_use_channel(s->type) : channel;

    if (ch < 0 || channel_in_use(ch) == 1)
        return S3D_PLAY_ERR_NO_CHANNEL;

    float v = (float)volume / (float)MAX_VOLUME;

#if defined(PLATFORM_PC)

    Mix_PlayChannel(ch, s->chunk, repeats ? repeats - 1 : -1);
    s->playing = true;
    s->channel = ch;

    Mix_Volume(ch, (int)(v * MIX_MAX_VOLUME));

#elif defined(PLATFORM_3DS)

    // ----------------------------
    // RESET STATE CLEAN
    // ----------------------------
    s->repeats = repeats;
    s->finished = false;
    s->channel = ch;
    s->playing = false;

    s->currentBuffer = 0;

    ndspChnReset(ch);

    ndspChnSetInterp(ch, NDSP_INTERP_LINEAR);
    ndspChnSetRate(ch, 48000);
    ndspChnSetFormat(ch, NDSP_FORMAT_STEREO_PCM16);

    float mix[12] = {0};
    mix[0] = v;
    mix[1] = v;
    ndspChnSetMix(ch, mix);
    
    // ----------------------------
    // MONO -> STEREO FIX (CORRECTO)
    // ----------------------------
    if (s->pcmChannels == 1)
    {
        short *src = s->pcmBuffer[0];
        int samples = OPUS_CHUNK_FRAMES;

        for (int i = samples - 1; i >= 0; i--)
        {
            short v = src[i];
            src[i * 2 + 0] = v;
            src[i * 2 + 1] = v;
        }

        s->pcmChannels = 2;
    }

    // ----------------------------
    // FLUSH CORRECTO
    // ----------------------------
    DSP_FlushDataCache(
        s->pcmBuffer[0],
        OPUS_CHUNK_FRAMES * s->pcmChannels * sizeof(short)
    );

    DSP_FlushDataCache(
        s->pcmBuffer[1],
        OPUS_CHUNK_FRAMES * s->pcmChannels * sizeof(short)
    );

    // ----------------------------
    // BUFFER 0
    // ----------------------------
    int frames0 = fill_opus_chunk(s, 0);
    if (frames0 <= 0)
        return S3D_PLAY_ERR_NO_SOUND;

    memset(&s->waveBufs[0], 0, sizeof(ndspWaveBuf));

    s->waveBufs[0].data_vaddr = s->pcmBuffer[0];
    s->waveBufs[0].nsamples = frames0;
    s->waveBufs[0].looping = false;

    ndspChnWaveBufAdd(ch, &s->waveBufs[0]);

    // ----------------------------
    // BUFFER 1 (PRELOAD)
    // ----------------------------
    int frames1 = fill_opus_chunk(s, 1);

    if (frames1 > 0)
    {
        memset(&s->waveBufs[1], 0, sizeof(ndspWaveBuf));

        s->waveBufs[1].data_vaddr = s->pcmBuffer[1];
        s->waveBufs[1].nsamples = frames1;
        s->waveBufs[1].looping = false;

        ndspChnWaveBufAdd(ch, &s->waveBufs[1]);
    }

    // ----------------------------
    // FINAL STATE
    // ----------------------------
    s->playing = true;

    used_channels[ch] = true;
    sound_channels[ch] = s;

    return S3D_PLAY_SUCCESS;

#endif

    return S3D_PLAY_SUCCESS;
}

void pause_any(S3D_Sound *s)
{
    if(!s->playing)
        return;
#if defined(PLATFORM_PC)
    Mix_Pause(s->channel);
#elif defined(PLATFORM_3DS)
    ndspChnSetPaused(s->channel, true);
#endif
    s->playing = false;
}
void continue_any(S3D_Sound *s)
{
    if(s->playing)
        return;
#if defined(PLATFORM_PC)
    Mix_Resume(s->channel);
#elif defined(PLATFORM_3DS)
    ndspChnSetPaused(s->channel, false);
#endif
    s->playing = true;
}
void stop_any(S3D_Sound *s)
{
#if defined(PLATFORM_PC)
    Mix_HaltChannel(s->channel);
    Mix_Volume(s->channel, MIX_MAX_VOLUME);
    Mix_UnregisterAllEffects(s->channel);
#elif defined(PLATFORM_3DS)
    ndspChnReset(s->channel);
    float mixVolume[12] = {1.0f, 1.0f};
    ndspChnSetMix(s->channel, mixVolume);
#endif
    s->playing = false;
    used_channels[s->channel] = false;
    sound_channels[s->channel] = NULL;
    s->channel = -1;
}

void s3d_init()
{
    if(initialized)
        return;
    initialized = true;
    s3d_set_master_volume(100);
    s3d_set_sound_volume(100);
    s3d_set_music_volume(100);
    s3d_set_voice_volume(100);
    s3d_set_ambi_volume(100);

    for (int i = 0; i < CHANNELS; i++)
    {
        used_channels[i] = false;
    }

    for (int i = 0; i < CHANNELS; i++)
    {
        sound_channels[i] = NULL;
    }
    printf("Initializating S3D.\n");

#if defined(PLATFORM_PC)
    if (Mix_Init(MIX_INIT_OGG | MIX_INIT_MP3) == 0)
    {
        printf("Mix_Init error: %s\n", Mix_GetError());
        return;
    }

    if (Mix_OpenAudio(AUDIO_SAMPLE_RATE, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("Mix_OpenAudio error: %s\n", Mix_GetError());
        return;
    }

    Mix_AllocateChannels(CHANNELS);
    Mix_ReserveChannels(CHANNELS);
#elif defined(PLATFORM_3DS)
    ndspInit();
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    for (int i = 0; i < CHANNELS; i++)
    {
        ndspChnReset(i);
        ndspChnSetInterp(i, NDSP_INTERP_LINEAR);
    }
#endif
}

void s3d_end_frame()
{
}

void free_any_sound(S3D_Sound *s)
{
    if (!s) return;

    printf("[FREE] sound\n");

#if defined(PLATFORM_PC)
    if(s->channel >= 0)
    {
        used_channels[s->channel] = false;
        sound_channels[s->channel] = NULL;
        Mix_HaltChannel(s->channel);
        s->channel = 0;
    }
    Mix_FreeChunk(s->chunk);
#elif defined(PLATFORM_3DS)
    if (s->channel >= 0)
    {
        used_channels[s->channel] = false;
        sound_channels[s->channel] = NULL;
        ndspChnReset(s->channel);
        s->channel = -1;
    }

    if (s->opus)
    {
        op_free(s->opus);
        s->opus = NULL;
    }

    if (s->pcmBuffer)
    {
        linearFree(s->pcmBuffer);
        s->pcmBuffer[0] = NULL;
        s->pcmBuffer[1] = NULL;
    }
#endif

    free(s);
}

void s3d_free_sound(S3D_Sound *sound)
{
    if(sound->type != SOUND_TYPE)
        return;
    free_any_sound(sound);
}
void s3d_free_music(S3D_Music *music)
{
    if(music->type != MUSIC_TYPE)
        return;
    free_any_sound(music);
}
void s3d_free_voice(S3D_Voice *voice)
{
    if(voice->type != VOICE_TYPE)
        return;
    free_any_sound(voice);
}
void s3d_free_ambi(S3D_Ambi *ambi)
{
    if(ambi->type != AMBI_TYPE)
        return;
    free_any_sound(ambi);
}

#if defined(PLATFORM_3DS)
void update_channel(int ch)
{
    S3D_Sound *s = sound_channels[ch];
    if (!s || !s->playing) return;

    for (int i = 0; i < 2; i++)
    {
        ndspWaveBuf *wb = &s->waveBufs[i];

        // si el buffer sigue en cola o sonando, no lo reutilices
        if (wb->status != NDSP_WBUF_DONE && wb->status != NDSP_WBUF_FREE)
            continue;

        int frames = fill_opus_chunk(s, i);

        if (frames <= 0)
        {
            ndspChnReset(ch);
            s->playing = false;
            s->channel = -1;
            used_channels[ch] = false;
            sound_channels[ch] = NULL;
            return;
        }

        memset(wb, 0, sizeof(ndspWaveBuf));

        wb->data_vaddr = s->pcmBuffer[i];
        wb->nsamples = frames;
        wb->looping = false;

        ndspChnWaveBufAdd(ch, wb);
    }
}
#endif

void s3d_begin_frame()
{
    for (int i = 0; i < CHANNELS; i++)
    {
#if defined(PLATFORM_3DS)
        bool playing = ndspChnIsPlaying(i);
#else
        bool playing = Mix_Playing(i);
#endif

        S3D_Sound *s = sound_channels[i];

        if (!s || !used_channels[i])
            continue;

#if defined(PLATFORM_3DS)

        update_channel(i);
#elif defined(PLATFORM_PC)
        if(!s->playing && s->channel >= 0)
        {
            used_channels[i] = false;
            sound_channels[i] = NULL;
            Mix_HaltChannel(s->channel);
            s->channel = -1;
        }
#endif
    }
}

#if defined(PLATFORM_PC)
Sint64 SDLCALL PAK_seek(SDL_RWops *context, Sint64 offset, int whence) {
    PAK_FILE* f = (PAK_FILE*)context->hidden.unknown.data1;
    if (PAKL_fseek(f, (long)offset, whence) == 0) {
        return (Sint64)PAKL_ftell(f);
    }
    return -1; // Error
}

// Callback para leer datos (read)
size_t SDLCALL PAK_read(SDL_RWops *context, void *ptr, size_t size, size_t maxnum) {
    PAK_FILE* f = (PAK_FILE*)context->hidden.unknown.data1;
    return PAKL_fread(ptr, size, maxnum, f);
}

// Callback para cerrar el archivo (close)
int SDLCALL PAK_close(SDL_RWops *context) {
    if (context) {
        PAK_FILE* f = (PAK_FILE*)context->hidden.unknown.data1;
        if (f) {
            PAKL_CloseFile(f);
        }
        SDL_FreeRW(context);
    }
    return 0;
}

// Nota: Mix_LoadWAV_RW no necesita escribir, implementamos un callback vacío
size_t SDLCALL PAK_write(SDL_RWops *context, const void *ptr, size_t size, size_t num) {
    return 0; 
}

SDL_RWops* SDL_RWFromPAK(const char* filename) {
    PAK_FILE* f = PAKL_LoadFile(filename);
    if (!f) return NULL;

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

    SDL_RWops* rw = SDL_RWFromMem((void*)buffer, tamano);
    if (!rw) {
        PAKL_CloseFile(f);
        return NULL;
    }

    return rw;
}

Mix_Chunk *GetChunkPC(const char* path)
{
    Mix_Chunk *chunk = NULL;
    SDL_RWops* rw = SDL_RWFromPAK(path);

    if (rw != NULL) {
        // El '1' final le dice a SDL_mixer que llame automáticamente a PAK_close al terminar
        chunk = Mix_LoadWAV_RW(rw, 1); 
    }

    return chunk;
}

#endif

S3D_Sound *MakeAny(const char *path, int channels, int bits)
{
    S3D_Sound *sound = malloc(sizeof(S3D_Sound));

    if (!sound)
        return NULL;

#if defined(PLATFORM_3DS)
    if (!CargarAudioOpus(path, sound))
    {
        free(sound);
        return NULL;
    }
#elif defined(PLATFORM_PC)
    sound->chunk = GetChunkPC(path);
    
    if(!sound->chunk){
        free(sound);
        return NULL;
    }
#endif
    sound->channel = -1;
    sound->playing = false;

    return sound;
}

S3D_Sound *s3d_make_any_of(const char *path){
    
    char p[512];
#if defined(PLATFORM_PC)
    int n = snprintf(p, sizeof(p), "%s", path);
#elif defined(PLATFORM_3DS)
    int n =snprintf(p, sizeof(p), "romfs:/%s.opus", path);
#else
    int n = -1;
#endif
    if (n >= sizeof(p)) {
        return NULL;
    }
    S3D_Sound *s = MakeAny(p, 2, 16);
    if (!s)
        return NULL;
    s->type = SOUND_TYPE;
    return s;
}

S3D_Sound *s3d_make_sound(const char *path)
{
    return s3d_make_any_of(path);
}

int s3d_play_sound(S3D_Sound *sound, int volume, int channel, int repeats)
{
    if(sound->type != SOUND_TYPE)
        return -1;
    int p = play_any(sound, volume, channel, repeats);

    return p;
}
void s3d_pause_sound(S3D_Sound *sound)
{
    pause_any(sound);
}
void s3d_continue_sound(S3D_Sound *sound)
{
    continue_any(sound);
}
void s3d_stop_sound(S3D_Sound *sound)
{
    stop_any(sound);
}

S3D_Music *s3d_make_music(const char *path)
{
    return s3d_make_any_of(path);
}

int s3d_play_music(S3D_Sound *music, int volume, int channel, int repeats)
{
    if(music->type != MUSIC_TYPE)
        return -1;
    int p = play_any(music, volume, channel, repeats);

    return p;
}

S3D_Voice *s3d_make_voice(const char *path)
{
    return s3d_make_any_of(path);
}

int s3d_play_voice(S3D_Voice *voice, int volume, int channel, int repeats)
{
    if(voice->type != VOICE_TYPE)
        return -1;
    int p = play_any(voice, volume, channel, repeats);

    return p;
}

S3D_Ambi *s3d_make_ambi(const char *path)
{
    return s3d_make_any_of(path);
}

int s3d_play_ambi(S3D_Ambi *ambi, int volume, int channel, int repeats)
{
    if(ambi->type != AMBI_TYPE)
        return -1;
    int p = play_any(ambi, volume, channel, repeats);

    return p;
}

void s3d_exit()
{
    if(!initialized)
        return;
    initialized = false;
#if defined(PLATFORM_3DS)
    ndspExit();
#elif defined(PLATFORM_PC)
    Mix_CloseAudio();
    Mix_Quit();
#endif
}