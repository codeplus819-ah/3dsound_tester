#include "scanner.h"
#include <filesystem>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

namespace scanner {

std::vector<std::string> scan(const std::string& folderPath) {
    std::vector<std::string> result;

    if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
        std::cerr << "[scanner] Did not find folder \"" << folderPath << "\"\n";
        return result;
    }

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (!entry.is_regular_file()) continue;

        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".wav") {
            result.push_back(entry.path().string());
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}

}