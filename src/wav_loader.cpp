#include "wav_loader.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace wav {

static bool matchTag(const char* data, const char* tag) {
    return std::memcmp(data, tag, 4) == 0;
}

static std::vector<int16_t> convertToInt16(
    const std::vector<uint8_t>& raw,
    int bitsPerSample,
    int audioFormat)
{
    std::vector<int16_t> out;

    if (audioFormat == 1) {
        if (bitsPerSample == 8) {
            out.resize(raw.size());
            for (size_t i = 0; i < raw.size(); ++i) {
                int16_t s = (static_cast<int16_t>(raw[i]) - 128) * 256;
                out[i] = s;
            }
        }
        else if (bitsPerSample == 16) {
            size_t n = raw.size() / 2;
            out.resize(n);
            for (size_t i = 0; i < n; ++i) {
                out[i] = static_cast<int16_t>(
                    raw[i * 2] | (raw[i * 2 + 1] << 8));
            }
        }
        else if (bitsPerSample == 24) {
            size_t n = raw.size() / 3;
            out.resize(n);
            for (size_t i = 0; i < n; ++i) {
                int32_t v = raw[i * 3]
                          | (raw[i * 3 + 1] << 8)
                          | (raw[i * 3 + 2] << 16);
                if (v & 0x800000) v |= 0xFF000000;
                out[i] = static_cast<int16_t>(v >> 8);
            }
        }
        else if (bitsPerSample == 32) {
            size_t n = raw.size() / 4;
            out.resize(n);
            for (size_t i = 0; i < n; ++i) {
                int32_t v = raw[i * 4]
                          | (raw[i * 4 + 1] << 8)
                          | (raw[i * 4 + 2] << 16)
                          | (raw[i * 4 + 3] << 24);
                out[i] = static_cast<int16_t>(v >> 16);
            }
        }
    }
    else if (audioFormat == 3) {
        if (bitsPerSample == 32) {
            size_t n = raw.size() / 4;
            out.resize(n);
            for (size_t i = 0; i < n; ++i) {
                uint32_t u = static_cast<uint32_t>(raw[i * 4])
                           | (static_cast<uint32_t>(raw[i * 4 + 1]) << 8)
                           | (static_cast<uint32_t>(raw[i * 4 + 2]) << 16)
                           | (static_cast<uint32_t>(raw[i * 4 + 3]) << 24);
                float f;
                std::memcpy(&f, &u, sizeof(float));
                f = std::clamp(f, -1.0f, 1.0f);
                out[i] = static_cast<int16_t>(f * 32767.0f);
            }
        }
        else if (bitsPerSample == 64) {
            size_t n = raw.size() / 8;
            out.resize(n);
            for (size_t i = 0; i < n; ++i) {
                uint64_t u = 0;
                for (int b = 0; b < 8; ++b)
                    u |= static_cast<uint64_t>(raw[i * 8 + b]) << (b * 8);
                double d;
                std::memcpy(&d, &u, sizeof(double));
                d = std::clamp(d, -1.0, 1.0);
                out[i] = static_cast<int16_t>(d * 32767.0);
            }
        }
    }

    return out;
}

WavData load(const std::string& filepath) {
    WavData result;

    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "[wav loader] File could not open: " << filepath << "\n";
        return result;
    }

    char riff[4];
    file.read(riff, 4);
    if (!matchTag(riff, "RIFF")) {
        std::cerr << "[wav loader] Not RIFF: " << filepath << "\n";
        return result;
    }

    uint32_t fileSize;
    file.read(reinterpret_cast<char*>(&fileSize), 4);

    char wave[4];
    file.read(wave, 4);
    if (!matchTag(wave, "WAVE")) {
        std::cerr << "[wav loader] Not WAVE: " << filepath << "\n";
        return result;
    }

    int16_t  audioFormat   = 0;
    int16_t  numChannels   = 0;
    uint32_t sampleRate    = 0;
    int16_t  bitsPerSample = 0;
    std::vector<uint8_t> rawData;

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
            int16_t  blockAlign;
            file.read(reinterpret_cast<char*>(&byteRate), 4);
            file.read(reinterpret_cast<char*>(&blockAlign), 2);
            file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

            if (chunkSize > 16) {
                file.seekg(chunkSize - 16, std::ios::cur);
            }
        }
        else if (matchTag(chunkId, "data")) {
            rawData.resize(chunkSize);
            file.read(reinterpret_cast<char*>(rawData.data()), chunkSize);
        }
        else {
            file.seekg(chunkSize, std::ios::cur);
        }
    }

    if (rawData.empty() || numChannels <= 0) {
        std::cerr << "[wav loader] No data in: " << filepath << "\n";
        return result;
    }

    if (numChannels != 1 && numChannels != 2) {
        std::cerr << "[wav loader] Unsupported channel count ("
                  << numChannels << "): " << filepath << "\n";
        return result;
    }

    std::vector<int16_t> converted = convertToInt16(
        rawData, bitsPerSample, audioFormat);

    if (converted.empty()) {
        std::cerr << "[wav loader] Unsupported format ("
                  << audioFormat << " / "
                  << bitsPerSample << "-bit): " << filepath << "\n";
        return result;
    }

    result.samples     = std::move(converted);
    result.sampleRate  = sampleRate;
    result.channels    = numChannels;
    result.valid       = true;
    return result;
}

}