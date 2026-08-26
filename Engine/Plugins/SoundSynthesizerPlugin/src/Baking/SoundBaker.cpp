#include "SoundBaker.hpp"
#include "Core/Log.h"
#include "Utils/TEFileSystem.hpp"
#include <fstream>
#include <vector>

namespace SoundStudio
{

#pragma pack(push, 1)
struct WAVHeader
{
    uint32_t ChunkID = 0x46464952; // "RIFF"
    uint32_t ChunkSize = 0;
    uint32_t Format = 0x45564157; // "WAVE"

    uint32_t Subchunk1ID = 0x20746D66; // "fmt "
    uint32_t Subchunk1Size = 16;
    uint16_t AudioFormat = 1; // PCM
    uint16_t NumChannels = 2;
    uint32_t SampleRate = 44100;
    uint32_t ByteRate = 44100 * 2 * 2;
    uint16_t BlockAlign = 4;
    uint16_t BitsPerSample = 16;

    uint32_t Subchunk2ID = 0x61746164; // "data"
    uint32_t Subchunk2Size = 0;
};
#pragma pack(pop)

bool SoundBaker::BakeGraphToWAV(TERef<SoundGraph> graph, const SoundBakeOptions &options)
{
    if (!graph)
    {
        TE_CORE_ERROR("[SoundBaker] Cannot bake null SoundGraph.");
        return false;
    }

    TE_CORE_INFO("[SoundBaker] Baking sound graph to '{0}' (Duration: {1}s)...", options.OutputPath.c_str(),
                 options.DurationSeconds);

    SoundGraphEvaluator evaluator(graph);
    evaluator.Reset();

    uint32_t totalSamples = (uint32_t)(options.DurationSeconds * options.SampleRate);
    uint32_t totalBlocks = (totalSamples + AUDIO_BLOCK_SIZE - 1) / AUDIO_BLOCK_SIZE;
    uint32_t noteOffSample = (uint32_t)(options.TriggerDurationSeconds * options.SampleRate);

    TEArray<int16_t> pcmData;
    pcmData.reserve(totalBlocks * AUDIO_BLOCK_SIZE * options.Channels);

    evaluator.TriggerNote(options.TriggerMidiNote, 1.0f);
    bool noteOffSent = false;

    for (uint32_t b = 0; b < totalBlocks; ++b)
    {
        uint32_t currentSample = b * AUDIO_BLOCK_SIZE;
        if (!noteOffSent && currentSample >= noteOffSample)
        {
            evaluator.TriggerNoteOff();
            noteOffSent = true;
        }

        StereoAudioBlock stereo;
        evaluator.ProcessBlock(stereo);

        for (uint32_t s = 0; s < AUDIO_BLOCK_SIZE; ++s)
        {
            float l = std::clamp(stereo.Left.Samples[s], -1.0f, 1.0f);
            float r = std::clamp(stereo.Right.Samples[s], -1.0f, 1.0f);

            int16_t pcmL = (int16_t)(l * 32767.0f);
            int16_t pcmR = (int16_t)(r * 32767.0f);

            pcmData.push_back(pcmL);
            if (options.Channels == 2)
            {
                pcmData.push_back(pcmR);
            }
        }
    }

    // Write WAV file
    std::ofstream out(options.OutputPath.c_str(), std::ios::binary);
    if (!out.is_open())
    {
        TE_CORE_ERROR("[SoundBaker] Failed to open output file for writing: {0}", options.OutputPath.c_str());
        return false;
    }

    WAVHeader header;
    header.NumChannels = (uint16_t)options.Channels;
    header.SampleRate = options.SampleRate;
    header.BitsPerSample = 16;
    header.ByteRate = options.SampleRate * options.Channels * (header.BitsPerSample / 8);
    header.BlockAlign = (uint16_t)(options.Channels * (header.BitsPerSample / 8));
    header.Subchunk2Size = (uint32_t)(pcmData.size() * sizeof(int16_t));
    header.ChunkSize = 36 + header.Subchunk2Size;

    out.write((const char *)&header, sizeof(WAVHeader));
    out.write((const char *)pcmData.data(), pcmData.size() * sizeof(int16_t));
    out.close();

    TE_CORE_INFO("[SoundBaker] Successfully baked audio to '{0}' ({1} samples written).", options.OutputPath.c_str(),
                 totalSamples);
    return true;
}

} // namespace SoundStudio
