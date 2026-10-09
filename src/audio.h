#pragma once
#include <string>
#include <vector>
#include <AL/al.h>
#include <AL/alc.h>
#include "wav_loader.h"

namespace audio {

struct Sound {
    ALuint buffer = 0;
    ALuint source = 0;
    bool valid = false;
};

bool init();
void shutdown();

Sound createSound(const std::string& filepath);

void destroySound(Sound& s);

void play(Sound& s, float x, float y, float z, float delay = 0.0f);

void setListenerPosition(float x, float y, float z);
void setListenerOrientation(float fx, float fy, float fz,
                            float ux, float uy, float uz);

void update(float deltaTime);

}