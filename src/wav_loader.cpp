#include "wav_loader.h"
#include <fstream>
#include <iostream>
#include <cstring>

namespace wav {

static bool matchTag(const char* data, const char* tag) {
    return std::memcmp(data, tag, 4) == 0;
}

WavData load(const std::string& filepath) {
    WavData result;

    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "[wav loader] Did not find file \"" << filepath << "\"\n";
        return result;
    }

    char riff[4];
    file.read(riff, 4);
    if (!matchTag(riff, "RIFF")) {
        std::cerr << "[wav loader] No RIFF\n";
        return result;
    }

    uint32_t fileSize;
    file.read(reinterpret_cast<char*>(&fileSize), 4);

    char wave[4];
    file.read(wave, 4);
    if (!matchTag(wave, "WAVE")) {
        std::cerr << "[wav loader] No wave\n";
        return result;
    }

    int16_t audioFormat = 0;
    int16_t numChannels = 0;
    uint32_t sampleRate = 0;
    int16_t bitsPerSample = 0;
    std::vector<int16_t> samples;

    while (file) {
        char chunkId[4];
        uint32_t chunkSize;
        file.read(chunkId, 4);
        file.read(reinterpret_cast<char*>(&chunkSize), 4);

        if (!file) break;

        if (matchTag(chunkId, "fmt ")) {
            file.read(reinterpret_cast<char*>(&audioFormat), 2);
            file.read(reinterpret_cast<char*>(&numChannels), 2);
            file.read(reinterpret_cast<char*>(&sampleRate), 4);

            uint32_t byteRate;
            int16_t blockAlign;
            file.read(reinterpret_cast<char*>(&byteRate), 4);
            file.read(reinterpret_cast<char*>(&blockAlign), 2);
            file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

            if (chunkSize > 16) {
                file.seekg(chunkSize - 16, std::ios::cur);
            }
        }
        else if (matchTag(chunkId, "data")) {
            if (audioFormat != 1 || bitsPerSample != 16) {
                std::cerr << "[wav loader] Just PCM 16-bit is supported\n";
                return result;
            }

            size_t numSamples = chunkSize / sizeof(int16_t);
            samples.resize(numSamples);
            file.read(reinterpret_cast<char*>(samples.data()), chunkSize);
            break;
        }
        else {
            file.seekg(chunkSize, std::ios::cur);
        }
    }

    if (samples.empty()) {
        std::cerr << "[wav loader] Did not find any data\n";
        return result;
    }

    result.samples = std::move(samples);
    result.sampleRate = sampleRate;
    result.channels = numChannels;
    result.valid = true;
    return result;
}

}