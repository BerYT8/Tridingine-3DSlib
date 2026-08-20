#if !defined(SOUND_S3D_H)
#define SOUND_S3D_H


typedef struct S3D_Sound S3D_Sound;

typedef S3D_Sound S3D_Ambi;
typedef S3D_Sound S3D_Music;
typedef S3D_Sound S3D_Voice;


/* ============================================================
 * SOUND TYPES
 * ============================================================ */

#define SOUND_TYPE 0
#define MUSIC_TYPE 1
#define VOICE_TYPE 2
#define AMBI_TYPE 3


/* ============================================================
 * PLAY RESULTS
 * ============================================================ */

/**
 * @brief Finalizado con éxito.
 */
#define S3D_PLAY_SUCCESS 0

/**
 * @brief No existe el sonido a reproducir o tiene un tipo inválido.
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


#ifdef __cplusplus
extern "C"
{
#endif


/* ============================================================
 * SYSTEM
 * ============================================================ */

/**
 * @brief Inicializa el sistema de audio.
 *
 * Debe ejecutarse antes de utilizar cualquier función de
 * reproducción o creación de sonidos.
 */
void s3d_init(void);


/**
 * @brief Actualiza el sistema de audio al comienzo del frame.
 *
 * En Nintendo 3DS esta función se encarga, entre otras cosas,
 * de mantener el streaming de los sonidos Opus.
 *
 * Debe llamarse una vez por frame.
 */
void s3d_begin_frame(void);


/**
 * @brief Finaliza la actualización del sistema de audio del frame.
 *
 * Debe llamarse una vez por frame después de s3d_begin_frame().
 */
void s3d_end_frame(void);


/**
 * @brief Cierra completamente el sistema de audio.
 *
 * Debe llamarse cuando ya no se vaya a utilizar el sistema
 * de sonido.
 */
void s3d_exit(void);


/* ============================================================
 * MASTER VOLUME
 * ============================================================ */

/**
 * @brief Establece el volumen maestro.
 *
 * @param volume Volumen entre 0 y 100.
 */
void s3d_set_master_volume(int volume);


/**
 * @brief Obtiene el volumen maestro actual.
 *
 * @return Volumen entre 0 y 100.
 */
int s3d_get_master_volume(void);


/* ============================================================
 * SOUND VOLUME
 * ============================================================ */

/**
 * @brief Establece el volumen de los efectos de sonido.
 *
 * @param volume Volumen entre 0 y 100.
 */
void s3d_set_sound_volume(int volume);


/**
 * @brief Obtiene el volumen de los efectos de sonido.
 *
 * @return Volumen entre 0 y 100.
 */
int s3d_get_sound_volume(void);


/* ============================================================
 * MUSIC VOLUME
 * ============================================================ */

/**
 * @brief Establece el volumen de la música.
 *
 * @param volume Volumen entre 0 y 100.
 */
void s3d_set_music_volume(int volume);


/**
 * @brief Obtiene el volumen de la música.
 *
 * @return Volumen entre 0 y 100.
 */
int s3d_get_music_volume(void);


/* ============================================================
 * VOICE VOLUME
 * ============================================================ */

/**
 * @brief Establece el volumen de las voces.
 *
 * @param volume Volumen entre 0 y 100.
 */
void s3d_set_voice_volume(int volume);


/**
 * @brief Obtiene el volumen de las voces.
 *
 * @return Volumen entre 0 y 100.
 */
int s3d_get_voice_volume(void);


/* ============================================================
 * AMBIENCE VOLUME
 * ============================================================ */

/**
 * @brief Establece el volumen de los sonidos ambientales.
 *
 * @param volume Volumen entre 0 y 100.
 */
void s3d_set_ambi_volume(int volume);


/**
 * @brief Obtiene el volumen de los sonidos ambientales.
 *
 * @return Volumen entre 0 y 100.
 */
int s3d_get_ambi_volume(void);


/* ============================================================
 * SOUND
 * ============================================================ */

/**
 * @brief Crea un efecto de sonido.
 *
 * @param path Ruta del sonido sin la extensión .opus.
 *
 * @return Puntero al sonido creado o NULL si ocurre un error.
 */
S3D_Sound *s3d_make_sound(
    const char *path
);


/**
 * @brief Libera un efecto de sonido.
 *
 * Si el sonido estaba reproduciéndose, se detiene antes de
 * liberar sus recursos.
 *
 * @param sound Efecto de sonido que se desea liberar.
 */
void s3d_free_sound(
    S3D_Sound *sound
);


/**
 * @brief Reproduce un efecto de sonido.
 *
 * @param sound Efecto de sonido.
 * @param volume Volumen solicitado entre 0 y 100.
 * @param channel Canal 0-23, o un valor negativo para seleccionar
 *                automáticamente un canal disponible.
 * @param repeats Número de reproducciones.
 *                0 = infinito.
 *                1 = una vez.
 *                2 = dos veces.
 *                etc.
 *
 * @return S3D_PLAY_SUCCESS si se inició correctamente.
 * @return Un código S3D_PLAY_ERR_* si ocurrió un error.
 */
int s3d_play_sound(
    S3D_Sound *sound,
    int volume,
    int channel,
    int repeats
);


/**
 * @brief Pausa un efecto de sonido.
 *
 * La posición actual de reproducción se conserva.
 *
 * Después de pausar, el sonido puede continuar desde la misma
 * posición utilizando s3d_continue_sound().
 *
 * @param sound Efecto de sonido que se desea pausar.
 */
void s3d_pause_sound(
    S3D_Sound *sound
);


/**
 * @brief Continúa un efecto de sonido pausado.
 *
 * La reproducción continúa desde la posición en la que fue
 * pausado.
 *
 * Si el sonido no está pausado, no realiza ninguna acción.
 *
 * @param sound Efecto de sonido que se desea continuar.
 */
void s3d_continue_sound(
    S3D_Sound *sound
);


/**
 * @brief Detiene completamente un efecto de sonido.
 *
 * A diferencia de s3d_pause_sound(), detener el sonido elimina
 * su reproducción actual y libera el canal utilizado.
 *
 * El objeto S3D_Sound sigue siendo válido y puede volver a
 * reproducirse posteriormente mediante s3d_play_sound().
 *
 * @param sound Efecto de sonido que se desea detener.
 */
void s3d_stop_sound(
    S3D_Sound *sound
);


/* ============================================================
 * MUSIC
 * ============================================================ */

/**
 * @brief Crea un recurso de música.
 *
 * @param path Ruta de la música sin la extensión .opus.
 *
 * @return Puntero a la música creada o NULL si ocurre un error.
 */
S3D_Music *s3d_make_music(
    const char *path
);


/**
 * @brief Libera un recurso de música.
 *
 * Si la música estaba reproduciéndose, se detiene antes de
 * liberar sus recursos.
 *
 * @param music Música que se desea liberar.
 */
void s3d_free_music(
    S3D_Music *music
);


/**
 * @brief Reproduce una música.
 *
 * @param music Música.
 * @param volume Volumen solicitado entre 0 y 100.
 * @param channel Canal 0-23, o un valor negativo para seleccionar
 *                automáticamente un canal disponible.
 * @param repeats Número de reproducciones.
 *                0 = infinito.
 *                1 = una vez.
 *                2 = dos veces.
 *                etc.
 *
 * @return S3D_PLAY_SUCCESS si se inició correctamente.
 * @return Un código S3D_PLAY_ERR_* si ocurrió un error.
 */
int s3d_play_music(
    S3D_Music *music,
    int volume,
    int channel,
    int repeats
);


/**
 * @brief Pausa una música.
 *
 * La posición actual de reproducción se conserva.
 *
 * La música puede continuar desde la misma posición mediante
 * s3d_continue_music().
 *
 * @param music Música que se desea pausar.
 */
void s3d_pause_music(
    S3D_Music *music
);


/**
 * @brief Continúa una música pausada.
 *
 * La reproducción continúa desde la posición en la que fue
 * pausada.
 *
 * Si la música no está pausada, no realiza ninguna acción.
 *
 * @param music Música que se desea continuar.
 */
void s3d_continue_music(
    S3D_Music *music
);


/**
 * @brief Detiene completamente una música.
 *
 * La reproducción actual se cancela y el canal utilizado queda
 * disponible para otros sonidos.
 *
 * El objeto S3D_Music sigue siendo válido y puede volver a
 * reproducirse posteriormente mediante s3d_play_music().
 *
 * @param music Música que se desea detener.
 */
void s3d_stop_music(
    S3D_Music *music
);


/* ============================================================
 * VOICE
 * ============================================================ */

/**
 * @brief Crea un recurso de voz.
 *
 * @param path Ruta de la voz sin la extensión .opus.
 *
 * @return Puntero a la voz creada o NULL si ocurre un error.
 */
S3D_Voice *s3d_make_voice(
    const char *path
);


/**
 * @brief Libera un recurso de voz.
 *
 * Si la voz estaba reproduciéndose, se detiene antes de
 * liberar sus recursos.
 *
 * @param voice Voz que se desea liberar.
 */
void s3d_free_voice(
    S3D_Voice *voice
);


/**
 * @brief Reproduce una voz.
 *
 * @param voice Voz.
 * @param volume Volumen solicitado entre 0 y 100.
 * @param channel Canal 0-23, o un valor negativo para seleccionar
 *                automáticamente un canal disponible.
 * @param repeats Número de reproducciones.
 *                0 = infinito.
 *                1 = una vez.
 *                2 = dos veces.
 *                etc.
 *
 * @return S3D_PLAY_SUCCESS si se inició correctamente.
 * @return Un código S3D_PLAY_ERR_* si ocurrió un error.
 */
int s3d_play_voice(
    S3D_Voice *voice,
    int volume,
    int channel,
    int repeats
);


/**
 * @brief Pausa una voz.
 *
 * La posición actual de reproducción se conserva.
 *
 * La voz puede continuar desde la misma posición mediante
 * s3d_continue_voice().
 *
 * @param voice Voz que se desea pausar.
 */
void s3d_pause_voice(
    S3D_Voice *voice
);


/**
 * @brief Continúa una voz pausada.
 *
 * La reproducción continúa desde la posición en la que fue
 * pausada.
 *
 * Si la voz no está pausada, no realiza ninguna acción.
 *
 * @param voice Voz que se desea continuar.
 */
void s3d_continue_voice(
    S3D_Voice *voice
);


/**
 * @brief Detiene completamente una voz.
 *
 * La reproducción actual se cancela y el canal utilizado queda
 * disponible para otros sonidos.
 *
 * El objeto S3D_Voice sigue siendo válido y puede volver a
 * reproducirse posteriormente mediante s3d_play_voice().
 *
 * @param voice Voz que se desea detener.
 */
void s3d_stop_voice(
    S3D_Voice *voice
);


/* ============================================================
 * AMBIENCE
 * ============================================================ */

/**
 * @brief Crea un recurso de sonido ambiental.
 *
 * @param path Ruta del sonido ambiental sin la extensión .opus.
 *
 * @return Puntero al sonido ambiental creado o NULL si ocurre
 *         un error.
 */
S3D_Ambi *s3d_make_ambi(
    const char *path
);


/**
 * @brief Libera un recurso de sonido ambiental.
 *
 * Si el sonido ambiental estaba reproduciéndose, se detiene
 * antes de liberar sus recursos.
 *
 * @param ambi Sonido ambiental que se desea liberar.
 */
void s3d_free_ambi(
    S3D_Ambi *ambi
);


/**
 * @brief Reproduce un sonido ambiental.
 *
 * @param ambi Sonido ambiental.
 * @param volume Volumen solicitado entre 0 y 100.
 * @param channel Canal 0-23, o un valor negativo para seleccionar
 *                automáticamente un canal disponible.
 * @param repeats Número de reproducciones.
 *                0 = infinito.
 *                1 = una vez.
 *                2 = dos veces.
 *                etc.
 *
 * @return S3D_PLAY_SUCCESS si se inició correctamente.
 * @return Un código S3D_PLAY_ERR_* si ocurrió un error.
 */
int s3d_play_ambi(
    S3D_Ambi *ambi,
    int volume,
    int channel,
    int repeats
);


/**
 * @brief Pausa un sonido ambiental.
 *
 * La posición actual de reproducción se conserva.
 *
 * El sonido puede continuar desde la misma posición mediante
 * s3d_continue_ambi().
 *
 * @param ambi Sonido ambiental que se desea pausar.
 */
void s3d_pause_ambi(
    S3D_Ambi *ambi
);


/**
 * @brief Continúa un sonido ambiental pausado.
 *
 * La reproducción continúa desde la posición en la que fue
 * pausado.
 *
 * Si el sonido ambiental no está pausado, no realiza ninguna
 * acción.
 *
 * @param ambi Sonido ambiental que se desea continuar.
 */
void s3d_continue_ambi(
    S3D_Ambi *ambi
);


/**
 * @brief Detiene completamente un sonido ambiental.
 *
 * La reproducción actual se cancela y el canal utilizado queda
 * disponible para otros sonidos.
 *
 * El objeto S3D_Ambi sigue siendo válido y puede volver a
 * reproducirse posteriormente mediante s3d_play_ambi().
 *
 * @param ambi Sonido ambiental que se desea detener.
 */
void s3d_stop_ambi(
    S3D_Ambi *ambi
);


#ifdef __cplusplus
}
#endif


#endif