#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace wav {

struct WavData {
    std::vector<int16_t> samples;
    int sampleRate = 0;
    int channels = 0;
    bool valid = false;
};

WavData load(const std::string& filepath);

}