#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <filesystem>
#include <iomanip>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#define STB_VORBIS_IMPLEMENTATION
#include "stb_vorbis.c"

#include "opusenc.h"
#include "ogg/ogg.h"

struct AudioData
{
    std::vector<short> samples;
    int channels;
    int sampleRate;
};

void printAudioInfo(
    const std::string &path,
    const AudioData &audio)
{
    namespace fs = std::filesystem;

    uintmax_t fileSize = 0;

    if (fs::exists(path))
    {
        fileSize = fs::file_size(path);
    }

    double duration =
        (double)audio.samples.size() /
        (audio.channels * audio.sampleRate);

    std::cout << "Información del archivo\n";
    std::cout << "----------------------\n";

    std::cout << "Archivo:      " << path << "\n";

    std::cout << "Formato:      "
              << path.substr(path.find_last_of('.') + 1)
              << "\n";

    std::cout << "Canales:      "
              << audio.channels
              << "\n";

    std::cout << "Sample Rate:  "
              << audio.sampleRate
              << " Hz\n";

    std::cout << "Samples:      "
              << audio.samples.size()
              << "\n";

    std::cout << "Duración:     "
              << std::fixed
              << std::setprecision(2)
              << duration
              << " segundos\n";

    std::cout << "Tamaño:       "
              << (fileSize / 1024.0)
              << " KB\n";
}

static std::string toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });
    return s;
}

static bool endsWith(const std::string &s, const std::string &suffix)
{
    if (s.size() < suffix.size())
        return false;

    return toLower(s.substr(s.size() - suffix.size())) == suffix;
}

bool loadWav(const std::string &path, AudioData &out)
{
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalPCMFrameCount;

    short *pSampleData = drwav_open_file_and_read_pcm_frames_s16(
        path.c_str(),
        &channels,
        &sampleRate,
        &totalPCMFrameCount,
        nullptr);

    if (!pSampleData)
        return false;

    out.channels = channels;
    out.sampleRate = sampleRate;

    out.samples.assign(
        pSampleData,
        pSampleData + totalPCMFrameCount * channels);

    drwav_free(pSampleData, nullptr);

    return true;
}

bool loadMp3(const std::string &path, AudioData &out)
{
    drmp3_config config;
    drmp3_int16 *pcmFrames;
    drmp3_uint64 frameCount;

    pcmFrames = drmp3_open_file_and_read_pcm_frames_s16(
        path.c_str(),
        &config,
        &frameCount,
        nullptr);

    if (!pcmFrames)
        return false;

    out.channels = config.channels;
    out.sampleRate = config.sampleRate;

    out.samples.assign(
        pcmFrames,
        pcmFrames + frameCount * config.channels);

    drmp3_free(pcmFrames, nullptr);

    return true;
}

bool loadOgg(const std::string &path, AudioData &out)
{
    int channels;
    int sampleRate;
    short *output;

    int samples = stb_vorbis_decode_filename(
        path.c_str(),
        &channels,
        &sampleRate,
        &output);

    if (samples < 0)
        return false;

    out.channels = channels;
    out.sampleRate = sampleRate;

    out.samples.assign(
        output,
        output + samples * channels);

    free(output);

    return true;
}

bool writeOpus(
    const std::string& filename,
    const AudioData& audio)
{
    int error;

    OggOpusComments* comments =
        ope_comments_create();

    if (!comments)
    {
        std::cout << "Error creando comentarios Opus\n";
        return false;
    }

    OggOpusEnc* enc =
        ope_encoder_create_file(
            filename.c_str(),
            comments,
            audio.sampleRate,
            audio.channels,
            0,
            &error);

    ope_comments_destroy(comments);

    if (error != OPE_OK || !enc)
    {
        std::cout << "Error creando encoder Opus: "
                  << ope_strerror(error)
                  << "\n";

        return false;
    }

    // Calidad/bitrate recomendado para 3DS
    ope_encoder_ctl(enc, OPUS_SET_BITRATE(96000));

    // VBR mejora calidad/tamaño
    ope_encoder_ctl(enc, OPUS_SET_VBR(1));

    // Audio complexity
    ope_encoder_ctl(enc, OPUS_SET_COMPLEXITY(5));

    // Resampling automático a 48kHz
    // libopusenc lo hace internamente

    const int frameSize = audio.sampleRate / 50;

    size_t totalFrames =
        audio.samples.size() / audio.channels;

    size_t cursor = 0;

    while (cursor < totalFrames)
    {
        size_t remain =
            totalFrames - cursor;

        int currentFrame =
            (remain >= frameSize)
                ? frameSize
                : (int)remain;

        std::vector<opus_int16> temp(
            frameSize * audio.channels,
            0);

        memcpy(
            temp.data(),
            &audio.samples[cursor * audio.channels],
            currentFrame *
                audio.channels *
                sizeof(short));

        error =
            ope_encoder_write(
                enc,
                temp.data(),
                frameSize);

        if (error != OPE_OK)
        {
            std::cout << "Error escribiendo Opus: "
                      << ope_strerror(error)
                      << "\n";

            ope_encoder_destroy(enc);
            return false;
        }

        cursor += currentFrame;
    }

    error = ope_encoder_drain(enc);

    if (error != OPE_OK)
    {
        std::cout << "Error finalizando Opus\n";

        ope_encoder_destroy(enc);
        return false;
    }

    ope_encoder_destroy(enc);

    return true;
}

bool copyFile(
    const std::string &src,
    const std::string &dst)
{
    std::ifstream in(src, std::ios::binary);

    if (!in)
        return false;

    std::ofstream out(dst, std::ios::binary);

    if (!out)
        return false;

    out << in.rdbuf();

    return true;
}

bool convertirArchivo(const std::string& inputFile,
                      const std::string& outputBase)
{
    AudioData audio;
    bool loaded = false;

    if (endsWith(inputFile, ".wav"))
        loaded = loadWav(inputFile, audio);
    else if (endsWith(inputFile, ".mp3"))
        loaded = loadMp3(inputFile, audio);
    else if (endsWith(inputFile, ".ogg"))
        loaded = loadOgg(inputFile, audio);
    else if (endsWith(inputFile, ".opus"))
    {
        return copyFile(inputFile, outputBase + ".opus");
    }
    else
    {
        return false;
    }

    if (!loaded)
        return false;

    return writeOpus(outputBase + ".opus", audio);
}

void mostrar_uso(const char* nombre)
{
    std::cout << "Uso:\n";
    std::cout << "  "  << nombre
                << " -i archivo -o output\n";
    std::cout << "  "  << nombre
                << " --all -i <carpeta> -o <carpeta>\n";
}

int main(int argc, char *argv[])
{
    std::string inputFile;
    std::string outputFile;

    bool all = false;

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "--help" && i + 1 < argc)
        {
            mostrar_uso(argv[0]);
            return 1;
        }
        if (arg == "--all")
        {
            all = true;
        }
        if (arg == "-i" && i + 1 < argc)
        {
            inputFile = argv[++i];
        }
        else if (arg == "-o" && i + 1 < argc)
        {
            outputFile = argv[++i];
        }
    }

    if (inputFile.empty())
    {
        mostrar_uso(argv[0]);
        return 1;
    }

    if (all)
    {
        namespace fs = std::filesystem;

        if (outputFile.empty())
        {
            std::cout << "Con --all es obligatorio usar -o\n";
            return 1;
        }

        fs::path inputDir(inputFile);
        fs::path outputDir(outputFile);

        if (!fs::exists(inputDir) || !fs::is_directory(inputDir))
        {
            std::cout << "-i debe ser una carpeta cuando se usa --all\n";
            return 1;
        }

        fs::create_directories(outputDir);

        for (const auto& entry : fs::recursive_directory_iterator(inputDir))
        {
            if (!entry.is_regular_file())
                continue;

            std::string ext = toLower(entry.path().extension().string());

            if (ext != ".wav" &&
                ext != ".mp3" &&
                ext != ".ogg" &&
                ext != ".opus")
                continue;

            fs::path relative =
                fs::relative(entry.path(), inputDir);

            fs::path out =
                outputDir / relative;

            out.replace_extension("");

            fs::create_directories(out.parent_path());

            if (convertirArchivo(
                    entry.path().string(),
                    out.string()))
            {
                std::cout << entry.path()
                        << " -> "
                        << out.string() << ".opus\n";
            }
            else
            {
                std::cout << "Error: "
                        << entry.path()
                        << "\n";
            }
        }

        return 0;
    }

    // ---------- modo normal ----------

    if (!convertirArchivo(inputFile, outputFile))
    {
        std::cout << "Error convirtiendo audio\n";
        return 1;
    }

    std::cout << "Conversión completada:\n";
    std::cout << outputFile << ".opus\n";

    return 0;
}