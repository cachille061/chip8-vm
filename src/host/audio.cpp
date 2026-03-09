#include "audio.hpp"

#include <cmath>
#include <cstring>
#include <numbers>
#include <stdexcept>
#include <string>

namespace chip8 {

Audio::Audio()
{
    SDL_AudioSpec want{};
    want.freq     = state_.sample_rate;
    want.format   = AUDIO_F32SYS;
    want.channels = 1;
    want.samples  = 512;
    want.callback = callback;
    want.userdata = &state_;

    SDL_AudioSpec have{};
    device_ = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (device_ == 0)
        throw std::runtime_error(std::string("SDL_OpenAudioDevice: ") + SDL_GetError());

    state_.sample_rate = have.freq;
}

Audio::~Audio()
{
    if (device_ != 0)
        SDL_CloseAudioDevice(device_);
}

void Audio::beep()
{
    if (!playing_) {
        SDL_PauseAudioDevice(device_, 0);   // unpause → start playback
        playing_ = true;
    }
}

void Audio::silence()
{
    if (playing_) {
        SDL_PauseAudioDevice(device_, 1);   // pause → stop playback
        playing_ = false;
    }
}

void Audio::callback(void* userdata, Uint8* stream, int len)
{
    auto* s = static_cast<State*>(userdata);
    auto* out = reinterpret_cast<float*>(stream);
    const int samples = len / static_cast<int>(sizeof(float));
    const float step = static_cast<float>(s->frequency) / static_cast<float>(s->sample_rate);

    for (int i = 0; i < samples; ++i) {
        // Square wave: positive half-cycle → +vol, negative → −vol
        out[i] = (s->phase < 0.5f) ? s->volume : -s->volume;
        s->phase += step;
        if (s->phase >= 1.0f)
            s->phase -= 1.0f;
    }
}

} // namespace chip8
