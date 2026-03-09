#pragma once

#include "core/types.hpp"

#include <SDL.h>
#include <array>
#include <cstdint>

namespace chip8 {

/// RAII wrapper around SDL window, renderer, and streaming texture.
/// Converts the 64×32 monochrome framebuffer into a scaled RGBA texture.
class Display {
public:
    explicit Display(int scale = 10);
    ~Display();

    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;

    /// Upload framebuffer to texture and present.
    void render(const std::array<std::uint8_t, DISPLAY_WIDTH * DISPLAY_HEIGHT>& fb);

    /// Update the window title bar.
    void set_title(const char* title);

private:
    SDL_Window*   window_   = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture*  texture_  = nullptr;
};

} // namespace chip8
