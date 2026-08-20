#if defined(PLATFORM_3DS)

#include "soundLoad.h"

#include <3ds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opusfile.h"


/* ============================================================
 * CONSTANTES / HELPERS
 * ============================================================ */


/*
 * Cada frame estéreo contiene:
 *
 *     L + R
 *
 * Por tanto:
 *
 *     frames * 2 * sizeof(short)
 */
static size_t opus_buffer_size(void)
{
    return
        (size_t)OPUS_CHUNK_FRAMES *
        2 *
        sizeof(short);
}


/* ============================================================
 * LOAD OPUS
 * ============================================================ */

bool CargarAudioOpus(
    const char *ruta,
    S3D_Sound *sound
)
{
    if (!ruta || !sound)
        return false;


    /*
     * Inicializar completamente la estructura.
     */
    memset(
        sound,
        0,
        sizeof(*sound)
    );


    sound->channel = -1;

    sound->playing = false;

    sound->currentBuffer = 0;

    sound->finished = false;

    sound->repeats = 0;


    printf(
        "[OPUS] Opening: %s\n",
        ruta
    );


    int error = 0;


    OggOpusFile *opus =
        op_open_file(
            ruta,
            &error
        );


    if (!opus)
    {
        printf(
            "[OPUS] op_open_file failed: %d\n",
            error
        );

        return false;
    }


    /*
     * Opusfile convierte la salida a estéreo.
     */
    sound->opus =
        opus;


    sound->pcmChannels =
        2;


    /*
     * 0 = loop infinito.
     */
    sound->repeats =
        0;


    /*
     * Todavía no hemos llegado al EOF.
     */
    sound->finished =
        false;


    /*
     * Limpiar punteros.
     */
    memset(
        sound->pcmBuffer,
        0,
        sizeof(sound->pcmBuffer)
    );


    /*
     * Limpiar WaveBuf.
     */
    memset(
        sound->waveBufs,
        0,
        sizeof(sound->waveBufs)
    );


    const size_t bufferSize =
        opus_buffer_size();


    /*
     * Reservar memoria lineal.
     */
    for (
        int i = 0;
        i < NDSP_QUEUED_BUFFERS;
        ++i
    )
    {
        sound->pcmBuffer[i] =
            (short *)linearAlloc(
                bufferSize
            );


        if (!sound->pcmBuffer[i])
        {
            printf(
                "[OPUS] linearAlloc failed: %d\n",
                i
            );


            /*
             * Liberar buffers ya reservados.
             */
            for (
                int j = 0;
                j < NDSP_QUEUED_BUFFERS;
                ++j
            )
            {
                if (sound->pcmBuffer[j])
                {
                    linearFree(
                        sound->pcmBuffer[j]
                    );

                    sound->pcmBuffer[j] =
                        NULL;
                }
            }


            op_free(
                opus
            );


            sound->opus =
                NULL;


            return false;
        }


        /*
         * No es necesario mantener este memset
         * durante la reproducción.
         *
         * Solamente se inicializa para dejar la
         * memoria limpia.
         */
        memset(
            sound->pcmBuffer[i],
            0,
            bufferSize
        );
    }


    printf(
        "[OPUS] Loaded\n"
        "       channels = 2\n"
        "       rate     = 48000\n"
        "       buffers  = %d\n"
        "       frames   = %d\n"
        "       ms/buf   = %.2f\n"
        "       total ms = %.2f\n"
        "       bytes    = %zu\n",
        NDSP_QUEUED_BUFFERS,
        OPUS_CHUNK_FRAMES,
        (
            (double)OPUS_CHUNK_FRAMES /
            (double)AUDIO_SAMPLE_RATE
        ) * 1000.0,
        (
            (double)OPUS_CHUNK_FRAMES *
            (double)NDSP_QUEUED_BUFFERS /
            (double)AUDIO_SAMPLE_RATE
        ) * 1000.0,
        bufferSize
    );


    return true;
}


/* ============================================================
 * FILL OPUS BUFFER
 * ============================================================ */

int fill_opus_chunk(
    S3D_Sound *sound,
    int bufferIndex
)
{
    if (!sound)
        return 0;


    if (!sound->opus)
        return 0;


    if (
        bufferIndex < 0 ||
        bufferIndex >= NDSP_QUEUED_BUFFERS
    )
        return 0;


    short *dst =
        sound->pcmBuffer[bufferIndex];


    if (!dst)
        return 0;


    /*
     * Si ya hemos alcanzado el EOF definitivo,
     * no volver a tocar el decoder.
     */
    if (sound->finished)
        return 0;


    /*
     * Leer hasta obtener audio válido o EOF.
     */
    for (;;)
    {
        /*
         * IMPORTANTE:
         *
         * op_read_stereo() devuelve número de frames
         * por canal.
         *
         * El tamaño del buffer se expresa como cantidad
         * de samples int16 disponibles.
         *
         * Estéreo:
         *
         *     OPUS_CHUNK_FRAMES * 2
         */
        int frames =
            op_read_stereo(
                sound->opus,
                dst,
                OPUS_CHUNK_FRAMES * 2
            );


        /*
         * ====================================================
         * AUDIO VÁLIDO
         * ====================================================
         */
        if (frames > 0)
        {
            /*
             * frames por canal.
             *
             * 2 canales.
             */
            size_t bytes =
                (size_t)frames *
                2 *
                sizeof(short);


            /*
             * Hacer visibles los datos al DSP.
             *
             * MUY IMPORTANTE en 3DS porque el buffer está
             * en memoria lineal y el DSP utiliza DMA.
             */
            DSP_FlushDataCache(
                dst,
                bytes
            );


            return frames;
        }


        /*
         * ====================================================
         * ERROR
         * ====================================================
         */
        if (frames < 0)
        {
            printf(
                "[OPUS] Decode error: %d\n",
                frames
            );


            sound->finished =
                true;


            return 0;
        }


        /*
         * ====================================================
         * EOF
         * ====================================================
         *
         * frames == 0
         */


        /*
         * ====================================================
         * LOOP INFINITO
         * ====================================================
         */
        if (sound->repeats == 0)
        {
            int seekResult =
                op_pcm_seek(
                    sound->opus,
                    0
                );


            if (seekResult < 0)
            {
                printf(
                    "[OPUS] Loop seek failed: %d\n",
                    seekResult
                );


                sound->finished =
                    true;


                return 0;
            }


            /*
             * Intentar volver a leer desde el principio.
             */
            continue;
        }


        /*
         * ====================================================
         * QUEDAN REPETICIONES
         * ====================================================
         */
        if (sound->repeats > 1)
        {
            --sound->repeats;


            int seekResult =
                op_pcm_seek(
                    sound->opus,
                    0
                );


            if (seekResult < 0)
            {
                printf(
                    "[OPUS] Repeat seek failed: %d\n",
                    seekResult
                );


                sound->finished =
                    true;


                return 0;
            }


            continue;
        }


        /*
         * ====================================================
         * ÚLTIMA REPRODUCCIÓN
         * ====================================================
         */
        sound->repeats =
            0;


        sound->finished =
            true;


        return 0;
    }
}

#endif