#pragma once

#include <SDL.h>

namespace chip8 {

/// RAII wrapper for SDL audio output.
/// Generates a 440 Hz square-wave beep, toggled with beep()/silence().
class Audio {
public:
    Audio();
    ~Audio();

    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    void beep();
    void silence();

private:
    SDL_AudioDeviceID device_ = 0;
    bool playing_ = false;

    static void callback(void* userdata, Uint8* stream, int len);

    // Shared state accessed from the audio callback.
    struct State {
        int   sample_rate   = 44100;
        int   frequency     = 440;
        float phase         = 0.0f;
        float volume        = 0.15f;   // ~15 % to avoid blasting the user
    } state_;
};

} // namespace chip8
