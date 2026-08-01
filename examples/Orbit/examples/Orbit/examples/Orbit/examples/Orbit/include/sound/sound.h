#if !defined(SOUND_S3D_H)
#define SOUND_S3D_H

typedef struct S3D_Sound S3D_Sound;
typedef S3D_Sound S3D_Ambi;
typedef S3D_Sound S3D_Music;
typedef S3D_Sound S3D_Voice;

#define SOUND_TYPE 0
#define MUSIC_TYPE 1
#define VOICE_TYPE 2
#define AMBI_TYPE 3

/**
 * @brief Finalizado con éxito.
 */
#define S3D_PLAY_SUCCESS 0
/**
 * @brief No existe el sonido a reproducir o este tiene un tipo inválido.
 */
#define S3D_PLAY_ERR_NO_SOUND -1
/**
 * @brief No existe formato para el tipo de sonido especificado.
 */
#define S3D_PLAY_ERR_FORMAT -2
/**
 * @brief No hay ningún canal libre en este momento.
 */
#define S3D_PLAY_ERR_NO_CHANNEL -4
/**
 * @brief Ha ocurrido un error inesperado.
 */
#define S3D_PLAY_ERR_PLATFORM -8
/**
 * @brief El sonido para ejecutar ya ha sido iniciado.
 */
#define S3D_PLAY_ERR_PLAYING_SOUND -16

typedef enum SoundFileType {
    SOUND_MP3,
    SOUND_WAV,
    SOUND_OGG
} SoundFileType;

#ifdef __cplusplus
extern "C"
{
#endif

    void s3d_init();

    void s3d_begin_frame();
    void s3d_end_frame();

    void s3d_set_master_volume(int volume);
    int s3d_get_master_volume();

    void s3d_set_sound_volume(int volume);
    int s3d_get_sound_volume();

    void s3d_set_music_volume(int volume);
    int s3d_get_music_volume();

    void s3d_set_voice_volume(int volume);
    int s3d_get_voice_volume();

    void s3d_set_ambi_volume(int volume);
    int s3d_get_ambi_volume();

    S3D_Sound *s3d_make_sound(const char *path);
    void s3d_free_sound(S3D_Sound *sound);

    /**
     * @brief Configura el hardware de audio y reproduce un sonido en un canal libre.
     *
     * @param sound Puntero a la estructura del sonido que se desea reproducir.
     * @param volume Valor del volumen entre 0 y 100 \b (valor \b restringido).
     * @param cannel Forzar canal entre 0-23 (otro número para canal automático).
     * @return int S3D_PLAY_SUCCESS (0) en caso de éxito, o un código de error negativo S3D_PLAY_ERR (< 0).
     */
    int s3d_play_sound(S3D_Sound *sound, int volume, int channel, int repeats);
    void s3d_pause_sound(S3D_Sound *sound);
    void s3d_continue_sound(S3D_Sound *sound);
    void s3d_stop_sound(S3D_Sound *sound);

    S3D_Music *s3d_make_music(const char *path);
    void s3d_free_music(S3D_Music *music);

    /**
     * @brief Configura el hardware de audio y reproduce un sonido en un canal libre.
     *
     * @param music Puntero a la estructura de la música que se desea reproducir.
     * @param volume Valor del volumen entre 0.0f y 1.0f \b (valor \b restringido).
     * @param cannel Forzar canal entre 0-23 (otro número para canal automático).
     * @return int S3D_PLAY_SUCCESS (0) en caso de éxito, o un código de error negativo S3D_PLAY_ERR (< 0).
     */
    int s3d_play_music(S3D_Music *music, int volume, int channel, int repeats);

    S3D_Voice *s3d_make_voice(const char *path);
    void s3d_free_voice(S3D_Voice *voice);

    /**
     * @brief Configura el hardware de audio y reproduce un sonido en un canal libre.
     *
     * @param voice Puntero a la estructura de la voz que se desea reproducir.
     * @param volume Valor del volumen entre 0.0f y 1.0f \b (valor \b restringido).
     * @param cannel Forzar canal entre 0-23 (otro número para canal automático).
     * @return int S3D_PLAY_SUCCESS (0) en caso de éxito, o un código de error negativo S3D_PLAY_ERR (< 0).
     */
    int s3d_play_voice(S3D_Voice *voice, int volume, int channel, int repeats);

    S3D_Ambi *s3d_make_ambi(const char *path);
    void s3d_free_ambi(S3D_Ambi *ambi);

    /**
     * @brief Configura el hardware de audio y reproduce un sonido en un canal libre.
     *
     * @param ambi Puntero a la estructura del sonido ambiente que se desea reproducir.
     * @param volume Valor del volumen entre 0.0f y 1.0f \b (valor \b restringido).
     * @param cannel Forzar canal entre 0-23 (otro número para canal automático).
     * @return int S3D_PLAY_SUCCESS (0) en caso de éxito, o un código de error negativo S3D_PLAY_ERR (< 0).
     */
    int s3d_play_ambi(S3D_Ambi *ambi, int volume, int channel, int repeats);

    void s3d_exit();

#ifdef __cplusplus
}
#endif

#endif