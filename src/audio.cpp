#ifndef ALC_SOFT_HRTF
#define ALC_SOFT_HRTF 1
#define ALC_HRTF_SOFT 0x1992
#endif

#include "audio.h"
#include <iostream>
#include <vector>
#include <algorithm>

namespace audio {

static ALCdevice* g_device = nullptr;
static ALCcontext* g_context = nullptr;

struct PendingSound {
    ALuint source;
    float timeLeft;
};

static std::vector<PendingSound> g_pending;

bool init() {
    g_device = alcOpenDevice(nullptr);
    if (!g_device) {
        std::cerr << "[audio player] Device OpenAL did not opened\n";
        return false;
    }

    ALCint attrs[] = {
        ALC_HRTF_SOFT, ALC_TRUE,
        0
    };
    g_context = alcCreateContext(g_device, attrs);
    if (!g_context) {
        std::cerr << "[audio player] Context did not created\n";
        alcCloseDevice(g_device);
        g_device = nullptr;
        return false;
    }

    alcMakeContextCurrent(g_context);

    std::cout << "[audio player] OpenAL: " << alGetString(AL_VENDOR) << "\n";
    std::cout << "[audio player] Renderer: " << alGetString(AL_RENDERER) << "\n";

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    return true;
}

void shutdown() {
    g_pending.clear();

    if (g_context) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(g_context);
        g_context = nullptr;
    }
    if (g_device) {
        alcCloseDevice(g_device);
        g_device = nullptr;
    }
}

Sound createSound(const std::string& filepath) {
    Sound s;

    wav::WavData data = wav::load(filepath);
    if (!data.valid) {
        std::cerr << "[audio player] File did not loaded: " << filepath << "\n";
        return s;
    }

    ALenum format;
    if (data.channels == 1)      format = AL_FORMAT_MONO16;
    else if (data.channels == 2) format = AL_FORMAT_STEREO16;
    else {
        std::cerr << "[audio player] Channels count is not supported: "
                  << data.channels << "\n";
        return s;
    }

    alGenBuffers(1, &s.buffer);
    alBufferData(s.buffer, format,
                 data.samples.data(),
                 static_cast<ALsizei>(data.samples.size() * sizeof(int16_t)),
                 data.sampleRate);

    alGenSources(1, &s.source);
    alSourcei(s.source, AL_BUFFER, s.buffer);

    alSourcef(s.source, AL_REFERENCE_DISTANCE, 1.0f);
    alSourcef(s.source, AL_MAX_DISTANCE, 100.0f);
    alSourcef(s.source, AL_ROLLOFF_FACTOR, 1.0f);

    s.valid = true;
    return s;
}

void destroySound(Sound& s) {
    if (s.source) {
        alDeleteSources(1, &s.source);
        s.source = 0;
    }
    if (s.buffer) {
        alDeleteBuffers(1, &s.buffer);
        s.buffer = 0;
    }
    s.valid = false;
}

void play(Sound& s, float x, float y, float z, float delay) {
    if (!s.valid) {
        std::cerr << "[audio player] Sound is not valid\n";
        return;
    }
    alSource3f(s.source, AL_POSITION, x, y, z);
    if (delay <= 0.0f) {
        alSourcePlay(s.source);
    } else {
        g_pending.erase(
            std::remove_if(g_pending.begin(), g_pending.end(),
                [&](const PendingSound& p) { return p.source == s.source; }),
            g_pending.end()
        );
        g_pending.push_back({ s.source, delay });
    }
}

void update(float deltaTime) {
    for (auto it = g_pending.begin(); it != g_pending.end(); ) {
        it->timeLeft -= deltaTime;
        if (it->timeLeft <= 0.0f) {
            alSourcePlay(it->source);
            it = g_pending.erase(it);
        } else {
            ++it;
        }
    }
}

void setListenerPosition(float x, float y, float z) {
    alListener3f(AL_POSITION, x, y, z);
}

void setListenerOrientation(float fx, float fy, float fz,
                            float ux, float uy, float uz) {
    float orient[6] = { fx, fy, fz, ux, uy, uz };
    alListenerfv(AL_ORIENTATION, orient);
}

}