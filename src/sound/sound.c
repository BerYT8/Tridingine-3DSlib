#include <sound/sound.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "soundLoad.h"

#include "../romfs_path.h"


#if defined(PLATFORM_PC)

#include <SDL.h>
#include <SDL_mixer.h>

#include <pak_loader/pak_loader.h>


#elif defined(PLATFORM_3DS)

#include <3ds.h>

#endif


/* ============================================================
 * GLOBAL
 * ============================================================ */

static bool soundInitialized = false;


/* ============================================================
 * VOLUMES
 * ============================================================ */

static int MasterVolume = MAX_VOLUME;
static int SoundVolume  = MAX_VOLUME;
static int MusicVolume  = MAX_VOLUME;
static int VoiceVolume  = MAX_VOLUME;
static int AmbiVolume   = MAX_VOLUME;


static int clamp_volume(
    int value
)
{
    if (value < 0)
        return 0;

    if (value > MAX_VOLUME)
        return MAX_VOLUME;

    return value;
}


void s3d_set_master_volume(
    int volume
)
{
    MasterVolume =
        clamp_volume(volume);
}


int s3d_get_master_volume(
    void
)
{
    return MasterVolume;
}


void s3d_set_sound_volume(
    int volume
)
{
    SoundVolume =
        clamp_volume(volume);
}


int s3d_get_sound_volume(
    void
)
{
    return SoundVolume;
}


void s3d_set_music_volume(
    int volume
)
{
    MusicVolume =
        clamp_volume(volume);
}


int s3d_get_music_volume(
    void
)
{
    return MusicVolume;
}


void s3d_set_voice_volume(
    int volume
)
{
    VoiceVolume =
        clamp_volume(volume);
}


int s3d_get_voice_volume(
    void
)
{
    return VoiceVolume;
}


void s3d_set_ambi_volume(
    int volume
)
{
    AmbiVolume =
        clamp_volume(volume);
}


int s3d_get_ambi_volume(
    void
)
{
    return AmbiVolume;
}


/* ============================================================
 * CHANNELS
 * ============================================================ */

static bool used_channels[CHANNELS];

static S3D_Sound *sound_channels[CHANNELS];


/*
 * Devuelve:
 *
 *  1 = ocupado
 *  0 = libre
 * -1 = canal inválido
 */
static int channel_in_use(
    int channel
)
{
    if (
        channel < 0 ||
        channel >= CHANNELS
    )
    {
        return -1;
    }


#if defined(PLATFORM_3DS)

    /*
     * used_channels es nuestra fuente principal.
     *
     * ndspChnIsPlaying() NO debe utilizarse para
     * decidir si un canal está libre porque un canal
     * pausado puede devolver false y aun así seguir
     * perteneciendo al sonido.
     */
    if (used_channels[channel])
        return 1;


    if (ndspChnIsPlaying(channel))
        return 1;


    return 0;


#elif defined(PLATFORM_PC)

    /*
     * Un canal pausado puede no considerarse "playing"
     * dependiendo de SDL_mixer, por lo que primero
     * comprobamos nuestra tabla.
     */
    if (used_channels[channel])
        return 1;


    if (Mix_Playing(channel))
        return 1;


    return 0;

#endif
}


/*
 * Obtener canal según categoría.
 */
static int get_next_use_channel(
    int type
)
{
    int first = 0;
    int last = CHANNELS - 1;


    switch (type)
    {
        case MUSIC_TYPE:

            first = 0;
            last = 1;

            break;


        case AMBI_TYPE:

            first = 2;
            last = 3;

            break;


        case VOICE_TYPE:

            first = 4;
            last = 5;

            break;


        case SOUND_TYPE:

            first = 6;
            last = CHANNELS - 1;

            break;


        default:

            printf(
                "[SOUND] Invalid type: %d\n",
                type
            );

            return -1;
    }


    for (
        int i = first;
        i <= last;
        ++i
    )
    {
        if (
            channel_in_use(i) == 0
        )
        {
            return i;
        }
    }


    return -1;
}


/* ============================================================
 * TYPE VOLUME
 * ============================================================ */

static int get_type_volume(
    int type
)
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

        default:
            return 0;
    }
}


/* ============================================================
 * CALCULATE VOLUME
 * ============================================================ */

static float calculate_volume(
    S3D_Sound *sound,
    int volume
)
{
    if (!sound)
        return 0.0f;


    int typeVolume =
        get_type_volume(
            sound->type
        );


    float finalVolume =
        ((float)volume / 100.0f) *
        ((float)typeVolume / 100.0f) *
        ((float)MasterVolume / 100.0f);


    if (finalVolume < 0.0f)
        finalVolume = 0.0f;


    if (finalVolume > 1.0f)
        finalVolume = 1.0f;


    return finalVolume;
}


/* ============================================================
 * PLAY
 * ============================================================ */

static int play_any(
    S3D_Sound *sound,
    int volume,
    int channel,
    int repeats
)
{
    if (!sound)
    {
        printf(
            "[SOUND] play_any: NULL sound\n"
        );

        return S3D_PLAY_ERR_NO_SOUND;
    }


    if (
        sound->type != SOUND_TYPE &&
        sound->type != MUSIC_TYPE &&
        sound->type != VOICE_TYPE &&
        sound->type != AMBI_TYPE
    )
    {
        printf(
            "[SOUND] Invalid sound type: %d\n",
            sound->type
        );

        return S3D_PLAY_ERR_NO_SOUND;
    }


    /*
     * No permitir dos reproducciones simultáneas
     * del mismo objeto.
     *
     * Si está pausado, también cuenta como activo.
     */
    if (sound->playing)
    {
        return S3D_PLAY_ERR_PLAYING_SOUND;
    }


    int ch;


    if (channel < 0)
    {
        ch =
            get_next_use_channel(
                sound->type
            );
    }
    else
    {
        ch = channel;
    }


    if (
        ch < 0 ||
        ch >= CHANNELS
    )
    {
        printf(
            "[SOUND] No channel for type %d\n",
            sound->type
        );

        return S3D_PLAY_ERR_NO_CHANNEL;
    }


    if (channel_in_use(ch))
    {
        printf(
            "[SOUND] Channel %d already in use\n",
            ch
        );

        return S3D_PLAY_ERR_NO_CHANNEL;
    }


    float finalVolume =
        calculate_volume(
            sound,
            volume
        );


#if defined(PLATFORM_PC)

    /* ========================================================
     * PC
     * ======================================================== */

    if (!sound->chunk)
    {
        printf(
            "[SOUND] play_any: NULL Mix_Chunk\n"
        );

        return S3D_PLAY_ERR_NO_SOUND;
    }


    /*
     * API:
     *
     * 0 = infinito
     * 1 = una vez
     * 2 = dos veces
     *
     * SDL_mixer:
     *
     * 0 = una vez
     * 1 = dos veces
     * -1 = infinito
     */
    int loops;


    if (repeats <= 0)
        loops = -1;
    else
        loops = repeats - 1;


    /*
     * Asegurarnos de que el canal no esté pausado
     * de una reproducción anterior.
     */
    Mix_Resume(
        ch
    );


    int result =
        Mix_PlayChannel(
            ch,
            sound->chunk,
            loops
        );


    if (result < 0)
    {
        printf(
            "[SOUND] Mix_PlayChannel failed: %s\n",
            Mix_GetError()
        );

        return S3D_PLAY_ERR_NO_CHANNEL;
    }


    Mix_Volume(
        ch,
        (int)(
            finalVolume *
            MIX_MAX_VOLUME
        )
    );


    sound->channel =
        ch;


    sound->playing =
        true;


    sound->paused =
        false;


    used_channels[ch] =
        true;


    sound_channels[ch] =
        sound;


    printf(
        "[SOUND] PC playing "
        "ch=%d type=%d\n",
        ch,
        sound->type
    );


    return S3D_PLAY_SUCCESS;


#elif defined(PLATFORM_3DS)

    /* ========================================================
     * 3DS
     * ======================================================== */

    if (
        sound->pcmChannels != 2 ||
        !sound->opus
    )
    {
        printf(
            "[SOUND] Invalid Opus sound\n"
        );

        return S3D_PLAY_ERR_NO_SOUND;
    }


    /*
     * IMPORTANTE:
     *
     * Cada nuevo PLAY empieza desde el principio.
     *
     * STOP -> PLAY
     * PLAY -> STOP -> PLAY
     *
     * siempre empiezan en PCM 0.
     */
    if (
        op_pcm_seek(
            sound->opus,
            0
        ) != 0
    )
    {
        printf(
            "[SOUND] op_pcm_seek(0) failed\n"
        );

        return S3D_PLAY_ERR_NO_SOUND;
    }


    /*
     * Configuración de repeticiones.
     */
    sound->repeats =
        repeats;


    sound->finished =
        false;


    sound->playing =
        false;


    sound->paused =
        false;


    sound->channel =
        ch;


    sound->currentBuffer =
        0;


    /*
     * ========================================================
     * CONFIGURAR NDSP
     * ========================================================
     */

    ndspChnReset(
        ch
    );


    ndspChnSetInterp(
        ch,
        NDSP_INTERP_LINEAR
    );


    ndspChnSetRate(
        ch,
        (float)AUDIO_SAMPLE_RATE
    );


    ndspChnSetFormat(
        ch,
        NDSP_FORMAT_STEREO_PCM16
    );


    /*
     * Volumen.
     */
    float mix[12];


    memset(
        mix,
        0,
        sizeof(mix)
    );


    mix[0] =
        finalVolume;


    mix[1] =
        finalVolume;


    ndspChnSetMix(
        ch,
        mix
    );


    /*
     * ========================================================
     * LIMPIAR WAVE BUFS
     * ========================================================
     *
     * Esto SOLO ocurre al iniciar una reproducción nueva.
     *
     * Nunca hacerlo durante PAUSE.
     */
    memset(
        sound->waveBufs,
        0,
        sizeof(sound->waveBufs)
    );


    /*
     * ========================================================
     * PREBUFFER
     * ========================================================
     */

    int queued = 0;


    for (
        int i = 0;
        i < NDSP_QUEUED_BUFFERS;
        ++i
    )
    {
        int frames =
            fill_opus_chunk(
                sound,
                i
            );


        if (frames <= 0)
        {
            /*
             * EOF.
             *
             * Los buffers ya encolados
             * podrán terminar.
             */
            break;
        }


        ndspWaveBuf *wb =
            &sound->waveBufs[i];


        wb->data_vaddr =
            sound->pcmBuffer[i];


        wb->nsamples =
            frames;


        wb->looping =
            false;


        ndspChnWaveBufAdd(
            ch,
            wb
        );


        ++queued;
    }


    /*
     * No se consiguió ningún buffer.
     */
    if (queued == 0)
    {
        ndspChnWaveBufClear(
            ch
        );


        ndspChnReset(
            ch
        );


        sound->channel =
            -1;


        sound->playing =
            false;


        sound->paused =
            false;


        sound->finished =
            false;


        return S3D_PLAY_ERR_NO_SOUND;
    }


    /*
     * Registrar canal.
     */
    sound->playing =
        true;


    sound->paused =
        false;


    used_channels[ch] =
        true;


    sound_channels[ch] =
        sound;


    printf(
        "[SOUND] 3DS playing "
        "ch=%d type=%d buffers=%d\n",
        ch,
        sound->type,
        queued
    );


    return S3D_PLAY_SUCCESS;

#endif
}


/* ============================================================
 * PAUSE
 * ============================================================ */

/**
 * @brief Pausa cualquier tipo de sonido.
 *
 * La reproducción queda pausada en la posición actual.
 * El canal continúa reservado para el sonido.
 *
 * Esta función es interna y permite pausar sonidos de cualquier
 * categoría: SOUND, MUSIC, VOICE o AMBI.
 *
 * @param sound Sonido que se desea pausar.
 */
static void pause_any_sound(
    S3D_Sound *sound
)
{
    if (!sound)
        return;


    /*
     * No hay reproducción activa.
     */
    if (!sound->playing)
        return;


    /*
     * Ya está pausado.
     */
    if (sound->paused)
        return;


    /*
     * Canal inválido.
     */
    if (
        sound->channel < 0 ||
        sound->channel >= CHANNELS
    )
    {
        return;
    }


    int ch =
        sound->channel;


#if defined(PLATFORM_PC)

    /*
     * SDL_mixer conserva la posición actual.
     *
     * NO utilizar Mix_HaltChannel(), ya que eso
     * detendría y reiniciaría la reproducción.
     */
    Mix_Pause(
        ch
    );


#elif defined(PLATFORM_3DS)

    /*
     * No limpiar ni resetear los WaveBuf.
     *
     * Los buffers permanecen en NDSP y la reproducción
     * continuará desde la posición actual.
     */
    ndspChnSetPaused(
        ch,
        true
    );

#endif


    /*
     * El sonido sigue activo, pero pausado.
     */
    sound->playing =
        true;


    sound->paused =
        true;


    /*
     * El canal sigue ocupado.
     */
    used_channels[ch] =
        true;


    printf(
        "[SOUND] Paused ch=%d type=%d\n",
        ch,
        sound->type
    );
}


/**
 * @brief Pausa un sonido normal.
 *
 * @param sound Sonido que se desea pausar.
 */
void s3d_pause_sound(
    S3D_Sound *sound
)
{
    if (!sound)
        return;


    if (sound->type != SOUND_TYPE)
        return;


    pause_any_sound(
        sound
    );
}


/**
 * @brief Pausa una música.
 *
 * @param music Música que se desea pausar.
 */
void s3d_pause_music(
    S3D_Music *music
)
{
    if (!music)
        return;


    if (music->type != MUSIC_TYPE)
        return;


    pause_any_sound(
        music
    );
}


/**
 * @brief Pausa una voz.
 *
 * @param voice Voz que se desea pausar.
 */
void s3d_pause_voice(
    S3D_Voice *voice
)
{
    if (!voice)
        return;


    if (voice->type != VOICE_TYPE)
        return;


    pause_any_sound(
        voice
    );
}


/**
 * @brief Pausa un sonido ambiental.
 *
 * @param ambi Sonido ambiental que se desea pausar.
 */
void s3d_pause_ambi(
    S3D_Ambi *ambi
)
{
    if (!ambi)
        return;


    if (ambi->type != AMBI_TYPE)
        return;


    pause_any_sound(
        ambi
    );
}


/* ============================================================
 * CONTINUE
 * ============================================================ */

/**
 * @brief Continúa cualquier tipo de sonido pausado.
 *
 * Reanuda la reproducción desde la posición exacta en la que
 * fue pausada.
 *
 * Esta función es interna y permite continuar sonidos de
 * cualquier categoría: SOUND, MUSIC, VOICE o AMBI.
 *
 * @param sound Sonido que se desea continuar.
 */
static void continue_any_sound(
    S3D_Sound *sound
)
{
    if (!sound)
        return;


    /*
     * Solo se puede continuar una reproducción activa
     * que esté actualmente pausada.
     */
    if (!sound->playing)
        return;


    if (!sound->paused)
        return;


    /*
     * Canal inválido.
     */
    if (
        sound->channel < 0 ||
        sound->channel >= CHANNELS
    )
    {
        sound->playing =
            false;

        sound->paused =
            false;

        return;
    }


    int ch =
        sound->channel;


#if defined(PLATFORM_PC)

    /*
     * Continúa desde la posición actual.
     */
    Mix_Resume(
        ch
    );


#elif defined(PLATFORM_3DS)

    /*
     * Los WaveBuf continúan en NDSP.
     *
     * La reproducción continúa desde donde se pausó.
     */
    ndspChnSetPaused(
        ch,
        false
    );

#endif


    sound->playing =
        true;


    sound->paused =
        false;


    used_channels[ch] =
        true;


    printf(
        "[SOUND] Continued ch=%d type=%d\n",
        ch,
        sound->type
    );
}


/**
 * @brief Continúa un sonido normal pausado.
 *
 * @param sound Sonido que se desea continuar.
 */
void s3d_continue_sound(
    S3D_Sound *sound
)
{
    if (!sound)
        return;


    if (sound->type != SOUND_TYPE)
        return;


    continue_any_sound(
        sound
    );
}


/**
 * @brief Continúa una música pausada.
 *
 * @param music Música que se desea continuar.
 */
void s3d_continue_music(
    S3D_Music *music
)
{
    if (!music)
        return;


    if (music->type != MUSIC_TYPE)
        return;


    continue_any_sound(
        music
    );
}


/**
 * @brief Continúa una voz pausada.
 *
 * @param voice Voz que se desea continuar.
 */
void s3d_continue_voice(
    S3D_Voice *voice
)
{
    if (!voice)
        return;


    if (voice->type != VOICE_TYPE)
        return;


    continue_any_sound(
        voice
    );
}


/**
 * @brief Continúa un sonido ambiental pausado.
 *
 * @param ambi Sonido ambiental que se desea continuar.
 */
void s3d_continue_ambi(
    S3D_Ambi *ambi
)
{
    if (!ambi)
        return;


    if (ambi->type != AMBI_TYPE)
        return;


    continue_any_sound(
        ambi
    );
}


/* ============================================================
 * STOP
 * ============================================================ */

/**
 * @brief Detiene cualquier tipo de sonido.
 *
 * La reproducción se detiene completamente y el canal utilizado
 * se libera. Si el sonido se vuelve a reproducir posteriormente,
 * comenzará desde el principio.
 *
 * Esta función es interna y permite detener sonidos de cualquier
 * categoría: SOUND, MUSIC, VOICE o AMBI.
 *
 * @param sound Sonido que se desea detener.
 */
static void stop_any(
    S3D_Sound *sound
)
{
    if (!sound)
        return;


    /*
     * Si no existe un canal activo, simplemente
     * limpiar el estado del sonido.
     */
    if (
        sound->channel < 0 ||
        sound->channel >= CHANNELS
    )
    {
        sound->playing =
            false;

        sound->paused =
            false;


#if defined(PLATFORM_3DS)

        sound->finished =
            false;

#endif

        return;
    }


    int ch =
        sound->channel;


#if defined(PLATFORM_PC)

    /*
     * Halt funciona tanto si está reproduciendo
     * como si está pausado.
     */
    Mix_HaltChannel(
        ch
    );


    /*
     * Restaurar el volumen del canal.
     */
    Mix_Volume(
        ch,
        MIX_MAX_VOLUME
    );


#elif defined(PLATFORM_3DS)

    /*
     * Eliminar todos los WaveBuf.
     */
    ndspChnWaveBufClear(
        ch
    );


    /*
     * Reset completo del canal.
     */
    ndspChnReset(
        ch
    );

#endif


    /*
     * Liberar el canal.
     */
    used_channels[ch] =
        false;


    if (
        sound_channels[ch] ==
        sound
    )
    {
        sound_channels[ch] =
            NULL;
    }


    /*
     * Estado detenido.
     */
    sound->playing =
        false;


    sound->paused =
        false;


    sound->channel =
        -1;


#if defined(PLATFORM_3DS)

    sound->finished =
        false;


    sound->currentBuffer =
        0;

#endif


    printf(
        "[SOUND] Stopped ch=%d type=%d\n",
        ch,
        sound->type
    );
}


/**
 * @brief Detiene un sonido normal.
 *
 * El canal se libera y una reproducción posterior comenzará
 * desde el principio.
 *
 * @param sound Sonido que se desea detener.
 */
void s3d_stop_sound(
    S3D_Sound *sound
)
{
    if (!sound)
        return;


    if (sound->type != SOUND_TYPE)
        return;


    stop_any(
        sound
    );
}


/**
 * @brief Detiene una música.
 *
 * El canal se libera y una reproducción posterior comenzará
 * desde el principio.
 *
 * @param music Música que se desea detener.
 */
void s3d_stop_music(
    S3D_Music *music
)
{
    if (!music)
        return;


    if (music->type != MUSIC_TYPE)
        return;


    stop_any(
        music
    );
}


/**
 * @brief Detiene una voz.
 *
 * El canal se libera y una reproducción posterior comenzará
 * desde el principio.
 *
 * @param voice Voz que se desea detener.
 */
void s3d_stop_voice(
    S3D_Voice *voice
)
{
    if (!voice)
        return;


    if (voice->type != VOICE_TYPE)
        return;


    stop_any(
        voice
    );
}


/**
 * @brief Detiene un sonido ambiental.
 *
 * El canal se libera y una reproducción posterior comenzará
 * desde el principio.
 *
 * @param ambi Sonido ambiental que se desea detener.
 */
void s3d_stop_ambi(
    S3D_Ambi *ambi
)
{
    if (!ambi)
        return;


    if (ambi->type != AMBI_TYPE)
        return;


    stop_any(
        ambi
    );
}


/* ============================================================
 * INIT
 * ============================================================ */

void s3d_init(
    void
)
{
    if (soundInitialized)
        return;


    printf(
        "[SOUND] Initializing...\n"
    );


    memset(
        used_channels,
        0,
        sizeof(used_channels)
    );


    memset(
        sound_channels,
        0,
        sizeof(sound_channels)
    );


#if defined(PLATFORM_PC)

    /* ========================================================
     * PC
     * ======================================================== */

    int requested =
        MIX_INIT_OPUS |
        MIX_INIT_OGG |
        MIX_INIT_MP3;


    int initialized =
        Mix_Init(
            requested
        );


    if (
        (initialized & MIX_INIT_OPUS)
        != MIX_INIT_OPUS
    )
    {
        printf(
            "[SOUND] SDL_mixer has no Opus support.\n"
            "[SOUND] Mix_Init returned: 0x%X\n"
            "[SOUND] Error: %s\n",
            initialized,
            Mix_GetError()
        );


        Mix_Quit();


        return;
    }


    if (
        Mix_OpenAudio(
            AUDIO_SAMPLE_RATE,
            MIX_DEFAULT_FORMAT,
            2,
            2048
        ) < 0
    )
    {
        printf(
            "[SOUND] Mix_OpenAudio failed: %s\n",
            Mix_GetError()
        );


        Mix_Quit();


        return;
    }


    int allocated =
        Mix_AllocateChannels(
            CHANNELS
        );


    Mix_ReserveChannels(
        0
    );


    printf(
        "[SOUND] SDL_mixer channels=%d\n",
        allocated
    );


#elif defined(PLATFORM_3DS)

    /* ========================================================
     * 3DS
     * ======================================================== */

    Result rc =
        ndspInit();


    if (R_FAILED(rc))
    {
        printf(
            "[SOUND] ndspInit failed: 0x%08lX\n",
            (unsigned long)rc
        );


        return;
    }


    ndspSetOutputMode(
        NDSP_OUTPUT_STEREO
    );


    for (
        int i = 0;
        i < CHANNELS;
        ++i
    )
    {
        ndspChnReset(
            i
        );


        ndspChnSetInterp(
            i,
            NDSP_INTERP_LINEAR
        );


        ndspChnSetRate(
            i,
            (float)AUDIO_SAMPLE_RATE
        );


        ndspChnSetFormat(
            i,
            NDSP_FORMAT_STEREO_PCM16
        );


        float mix[12];


        memset(
            mix,
            0,
            sizeof(mix)
        );


        mix[0] =
            1.0f;


        mix[1] =
            1.0f;


        ndspChnSetMix(
            i,
            mix
        );
    }

#endif


    MasterVolume = 100;
    SoundVolume  = 100;
    MusicVolume  = 100;
    VoiceVolume  = 100;
    AmbiVolume   = 100;


    soundInitialized =
        true;


    printf(
        "[SOUND] Initialized OK\n"
    );
}


/* ============================================================
 * END FRAME
 * ============================================================ */

void s3d_end_frame(
    void
)
{
}


/* ============================================================
 * 3DS STREAMING
 * ============================================================ */

#if defined(PLATFORM_3DS)


/*
 * ============================================================
 * CHANNEL HAS PENDING BUFFERS
 * ============================================================
 */

static bool channel_has_pending_buffers(
    S3D_Sound *sound
)
{
    if (!sound)
        return false;


    for (
        int i = 0;
        i < NDSP_QUEUED_BUFFERS;
        ++i
    )
    {
        ndspWaveBuf *wb =
            &sound->waveBufs[i];


        if (
            wb->status == NDSP_WBUF_QUEUED ||
            wb->status == NDSP_WBUF_PLAYING
        )
        {
            return true;
        }
    }


    return false;
}


/*
 * ============================================================
 * FINISH 3DS SOUND
 * ============================================================
 */

static void finish_3ds_sound(
    int channel,
    S3D_Sound *sound
)
{
    if (!sound)
        return;


    ndspChnWaveBufClear(
        channel
    );


    ndspChnReset(
        channel
    );


    used_channels[channel] =
        false;


    if (
        sound_channels[channel] ==
        sound
    )
    {
        sound_channels[channel] =
            NULL;
    }


    sound->playing =
        false;


    sound->paused =
        false;


    sound->channel =
        -1;


    sound->finished =
        false;


    sound->currentBuffer =
        0;
}


/*
 * ============================================================
 * UPDATE CHANNEL
 * ============================================================
 */

static void update_channel(
    int ch
)
{
    if (
        ch < 0 ||
        ch >= CHANNELS
    )
    {
        return;
    }


    S3D_Sound *sound =
        sound_channels[ch];


    if (!sound)
        return;


    /*
     * PAUSA:
     *
     * No rellenar buffers mientras está pausado.
     *
     * Los buffers que ya están en NDSP permanecen intactos.
     */
    if (
        !sound->playing ||
        sound->paused
    )
    {
        return;
    }


    /*
     * ========================================================
     * RELLENAR BUFFERS LIBRES
     * ========================================================
     */

    for (
        int i = 0;
        i < NDSP_QUEUED_BUFFERS;
        ++i
    )
    {
        ndspWaveBuf *wb =
            &sound->waveBufs[i];


        /*
         * NO TOCAR buffers activos.
         */
        if (
            wb->status == NDSP_WBUF_QUEUED ||
            wb->status == NDSP_WBUF_PLAYING
        )
        {
            continue;
        }


        /*
         * Si ya estamos en EOF definitivo,
         * no generar más audio.
         */
        if (sound->finished)
        {
            continue;
        }


        /*
         * El buffer está DONE/FREE.
         *
         * Ahora sí podemos reutilizarlo.
         */
        int frames =
            fill_opus_chunk(
                sound,
                i
            );


        if (frames <= 0)
        {
            /*
             * EOF o error.
             *
             * Los buffers que ya están en NDSP
             * terminan normalmente.
             */
            continue;
        }


        wb->data_vaddr =
            sound->pcmBuffer[i];


        wb->nsamples =
            frames;


        wb->looping =
            false;


        ndspChnWaveBufAdd(
            ch,
            wb
        );
    }


    /*
     * ========================================================
     * FINAL DE AUDIO
     * ========================================================
     */

    if (sound->finished)
    {
        bool pending =
            channel_has_pending_buffers(
                sound
            );


        /*
         * Esperar a que el último WaveBuf
         * termine completamente.
         */
        if (
            !pending &&
            !ndspChnIsPlaying(ch)
        )
        {
            finish_3ds_sound(
                ch,
                sound
            );
        }
    }
}


#endif


/* ============================================================
 * BEGIN FRAME
 * ============================================================ */

void s3d_begin_frame(
    void
)
{
    for (
        int i = 0;
        i < CHANNELS;
        ++i
    )
    {
        S3D_Sound *s =
            sound_channels[i];


        if (!s)
            continue;


        if (!used_channels[i])
            continue;


#if defined(PLATFORM_3DS)

        /*
         * Si está pausado, update_channel()
         * simplemente no toca los WaveBuf.
         */
        update_channel(
            i
        );


#elif defined(PLATFORM_PC)

        /*
         * ====================================================
         * PC
         * ====================================================
         *
         * MUY IMPORTANTE:
         *
         * Un sonido pausado tiene:
         *
         *     playing = true
         *     paused  = true
         *
         * Por tanto NO debemos detenerlo.
         */


        if (s->paused)
        {
            continue;
        }


        /*
         * Si sigue marcado como playing pero SDL_mixer
         * ya no lo está reproduciendo, significa que
         * terminó normalmente.
         */
        if (
            s->playing &&
            !Mix_Playing(
                s->channel
            )
        )
        {
            used_channels[i] =
                false;


            sound_channels[i] =
                NULL;


            s->playing =
                false;


            s->paused =
                false;


            s->channel =
                -1;


            continue;
        }

#endif
    }
}


/* ============================================================
 * PC LOAD FILE FROM PAK
 * ============================================================ */

#if defined(PLATFORM_PC)


static Mix_Chunk *GetChunkPC(
    const char *path
)
{
    if (!path)
        return NULL;


    printf(
        "[SOUND] Loading PC: %s\n",
        path
    );


    PAK_FILE *file =
        PAKL_LoadFile(
            path
        );


    if (!file)
    {
        printf(
            "[SOUND] PAKL_LoadFile failed: %s\n",
            path
        );


        return NULL;
    }


    if (
        PAKL_fseek(
            file,
            0,
            SEEK_END
        ) != 0
    )
    {
        PAKL_CloseFile(
            file
        );


        return NULL;
    }


    long size =
        PAKL_ftell(
            file
        );


    if (size <= 0)
    {
        PAKL_CloseFile(
            file
        );


        return NULL;
    }


    PAKL_rewind(
        file
    );


    void *buffer =
        malloc(
            (size_t)size
        );


    if (!buffer)
    {
        PAKL_CloseFile(
            file
        );


        return NULL;
    }


    size_t read =
        PAKL_fread(
            buffer,
            1,
            (size_t)size,
            file
        );


    PAKL_CloseFile(
        file
    );


    if (
        read !=
        (size_t)size
    )
    {
        printf(
            "[SOUND] Failed reading PAK: %s\n",
            path
        );


        free(
            buffer
        );


        return NULL;
    }


    SDL_RWops *rw =
        SDL_RWFromConstMem(
            buffer,
            (int)size
        );


    if (!rw)
    {
        free(
            buffer
        );


        return NULL;
    }


    Mix_Chunk *chunk =
        Mix_LoadWAV_RW(
            rw,
            1
        );


    free(
        buffer
    );


    if (!chunk)
    {
        printf(
            "[SOUND] Mix_LoadWAV_RW failed: %s\n",
            Mix_GetError()
        );


        return NULL;
    }


    printf(
        "[SOUND] PC audio loaded OK\n"
    );


    return chunk;
}


#endif


/* ============================================================
 * MAKE SOUND
 * ============================================================ */

static S3D_Sound *MakeAny(
    const char *path
)
{
    if (!path)
        return NULL;


    S3D_Sound *sound =
        (S3D_Sound *)calloc(
            1,
            sizeof(S3D_Sound)
        );


    if (!sound)
    {
        printf(
            "[SOUND] calloc failed\n"
        );


        return NULL;
    }


    sound->channel =
        -1;


    sound->playing =
        false;


    sound->paused =
        false;


#if defined(PLATFORM_3DS)

    if (
        !CargarAudioOpus(
            path,
            sound
        )
    )
    {
        printf(
            "[SOUND] Failed loading Opus: %s\n",
            path
        );


        free(
            sound
        );


        return NULL;
    }


#elif defined(PLATFORM_PC)

    sound->chunk =
        GetChunkPC(
            path
        );


    if (!sound->chunk)
    {
        printf(
            "[SOUND] Failed loading PC audio: %s\n",
            path
        );


        free(
            sound
        );


        return NULL;
    }

#endif


    return sound;
}


/* ============================================================
 * MAKE BY TYPE
 * ============================================================ */

static S3D_Sound *s3d_make_any_of(
    const char *path,
    int type
)
{
    if (!path)
        return NULL;


    char fullPath[512];


    int n =
        snprintf(
            fullPath,
            sizeof(fullPath),
            "%s.opus",
            path
        );


    if (
        n < 0 ||
        (size_t)n >= sizeof(fullPath)
    )
    {
        printf(
            "[SOUND] Path too long: %s\n",
            path
        );


        return NULL;
    }


    const char *realPath =
        getRomfsPath(
            fullPath
        );


    if (!realPath)
    {
        printf(
            "[SOUND] getRomfsPath returned NULL\n"
        );


        return NULL;
    }


    printf(
        "[SOUND] Creating type=%d path=%s\n",
        type,
        realPath
    );


    S3D_Sound *sound =
        MakeAny(
            realPath
        );


    if (!sound)
    {
        printf(
            "[SOUND] MakeAny FAILED type=%d path=%s\n",
            type,
            realPath
        );


        return NULL;
    }


    sound->type =
        type;


    sound->playing =
        false;


    sound->paused =
        false;


    sound->channel =
        -1;


    return sound;
}


/* ============================================================
 * PUBLIC MAKE
 * ============================================================ */

S3D_Sound *s3d_make_sound(
    const char *path
)
{
    return s3d_make_any_of(
        path,
        SOUND_TYPE
    );
}


S3D_Music *s3d_make_music(
    const char *path
)
{
    return s3d_make_any_of(
        path,
        MUSIC_TYPE
    );
}


S3D_Voice *s3d_make_voice(
    const char *path
)
{
    return s3d_make_any_of(
        path,
        VOICE_TYPE
    );
}


S3D_Ambi *s3d_make_ambi(
    const char *path
)
{
    return s3d_make_any_of(
        path,
        AMBI_TYPE
    );
}


/* ============================================================
 * PUBLIC PLAY
 * ============================================================ */

int s3d_play_sound(
    S3D_Sound *sound,
    int volume,
    int channel,
    int repeats
)
{
    if (!sound)
        return S3D_PLAY_ERR_NO_SOUND;


    if (sound->type != SOUND_TYPE)
        return S3D_PLAY_ERR_NO_SOUND;


    return play_any(
        sound,
        volume,
        channel,
        repeats
    );
}


int s3d_play_music(
    S3D_Music *music,
    int volume,
    int channel,
    int repeats
)
{
    if (!music)
        return S3D_PLAY_ERR_NO_SOUND;


    if (music->type != MUSIC_TYPE)
        return S3D_PLAY_ERR_NO_SOUND;


    return play_any(
        music,
        volume,
        channel,
        repeats
    );
}


int s3d_play_voice(
    S3D_Voice *voice,
    int volume,
    int channel,
    int repeats
)
{
    if (!voice)
        return S3D_PLAY_ERR_NO_SOUND;


    if (voice->type != VOICE_TYPE)
        return S3D_PLAY_ERR_NO_SOUND;


    return play_any(
        voice,
        volume,
        channel,
        repeats
    );
}


int s3d_play_ambi(
    S3D_Ambi *ambi,
    int volume,
    int channel,
    int repeats
)
{
    if (!ambi)
        return S3D_PLAY_ERR_NO_SOUND;


    if (ambi->type != AMBI_TYPE)
        return S3D_PLAY_ERR_NO_SOUND;


    return play_any(
        ambi,
        volume,
        channel,
        repeats
    );
}


/* ============================================================
 * FREE
 * ============================================================ */

static void free_any_sound(
    S3D_Sound *sound
)
{
    if (!sound)
        return;


    printf(
        "[SOUND] Free %p type=%d\n",
        (void *)sound,
        sound->type
    );


#if defined(PLATFORM_PC)

    /*
     * Detener reproducción si existe.
     */
    if (
        sound->channel >= 0 &&
        sound->channel < CHANNELS
    )
    {
        int ch =
            sound->channel;


        Mix_HaltChannel(
            ch
        );


        used_channels[ch] =
            false;


        if (
            sound_channels[ch] ==
            sound
        )
        {
            sound_channels[ch] =
                NULL;
        }


        sound->channel =
            -1;


        sound->playing =
            false;


        sound->paused =
            false;
    }


    /*
     * Liberar chunk.
     */
    if (sound->chunk)
    {
        Mix_FreeChunk(
            sound->chunk
        );


        sound->chunk =
            NULL;
    }


#elif defined(PLATFORM_3DS)

    /*
     * ========================================================
     * DETENER NDSP ANTES DE LIBERAR MEMORIA
     * ========================================================
     */

    if (
        sound->channel >= 0 &&
        sound->channel < CHANNELS
    )
    {
        int ch =
            sound->channel;


        ndspChnWaveBufClear(
            ch
        );


        ndspChnReset(
            ch
        );


        used_channels[ch] =
            false;


        if (
            sound_channels[ch] ==
            sound
        )
        {
            sound_channels[ch] =
                NULL;
        }


        sound->channel =
            -1;


        sound->playing =
            false;


        sound->paused =
            false;


        sound->finished =
            false;
    }


    /*
     * Liberar decoder.
     */
    if (sound->opus)
    {
        op_free(
            sound->opus
        );


        sound->opus =
            NULL;
    }


    /*
     * Liberar buffers lineales.
     */
    for (
        int i = 0;
        i < NDSP_QUEUED_BUFFERS;
        ++i
    )
    {
        if (sound->pcmBuffer[i])
        {
            linearFree(
                sound->pcmBuffer[i]
            );


            sound->pcmBuffer[i] =
                NULL;
        }
    }

#endif


    free(
        sound
    );
}


/* ============================================================
 * PUBLIC FREE
 * ============================================================ */

void s3d_free_sound(
    S3D_Sound *sound
)
{
    if (!sound)
        return;


    if (sound->type != SOUND_TYPE)
        return;


    free_any_sound(
        sound
    );
}


void s3d_free_music(
    S3D_Music *music
)
{
    if (!music)
        return;


    if (music->type != MUSIC_TYPE)
        return;


    free_any_sound(
        music
    );
}


void s3d_free_voice(
    S3D_Voice *voice
)
{
    if (!voice)
        return;


    if (voice->type != VOICE_TYPE)
        return;


    free_any_sound(
        voice
    );
}


void s3d_free_ambi(
    S3D_Ambi *ambi
)
{
    if (!ambi)
        return;


    if (ambi->type != AMBI_TYPE)
        return;


    free_any_sound(
        ambi
    );
}


/* ============================================================
 * EXIT
 * ============================================================ */

void s3d_exit(
    void
)
{
    if (!soundInitialized)
        return;


    printf(
        "[SOUND] Shutting down...\n"
    );


#if defined(PLATFORM_3DS)

    /*
     * Detener TODOS los canales antes de cerrar NDSP.
     */
    for (
        int i = 0;
        i < CHANNELS;
        ++i
    )
    {
        ndspChnWaveBufClear(
            i
        );


        ndspChnReset(
            i
        );


        used_channels[i] =
            false;


        sound_channels[i] =
            NULL;
    }


    /*
     * Limpiar callback.
     */
    ndspSetCallback(
        NULL,
        NULL
    );


    ndspExit();


#elif defined(PLATFORM_PC)

    Mix_HaltChannel(
        -1
    );


    Mix_CloseAudio();


    Mix_Quit();

#endif


    soundInitialized =
        false;


    printf(
        "[SOUND] Shutdown complete\n"
    );
}