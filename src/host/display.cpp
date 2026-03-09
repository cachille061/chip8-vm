#include "display.hpp"

#include <stdexcept>
#include <string>

namespace chip8 {

Display::Display(int scale)
{
    window_ = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        static_cast<int>(DISPLAY_WIDTH) * scale,
        static_cast<int>(DISPLAY_HEIGHT) * scale,
        SDL_WINDOW_SHOWN
    );
    if (!window_)
        throw std::runtime_error(std::string("SDL_CreateWindow: ") + SDL_GetError());

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_)
        throw std::runtime_error(std::string("SDL_CreateRenderer: ") + SDL_GetError());

    texture_ = SDL_CreateTexture(
        renderer_,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        DISPLAY_WIDTH, DISPLAY_HEIGHT
    );
    if (!texture_)
        throw std::runtime_error(std::string("SDL_CreateTexture: ") + SDL_GetError());
}

Display::~Display()
{
    if (texture_)  SDL_DestroyTexture(texture_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_)   SDL_DestroyWindow(window_);
}

void Display::render(
    const std::array<std::uint8_t, DISPLAY_WIDTH * DISPLAY_HEIGHT>& fb)
{
    // Convert monochrome framebuffer → RGBA pixels.
    std::array<std::uint32_t, DISPLAY_WIDTH * DISPLAY_HEIGHT> pixels{};
    for (std::size_t i = 0; i < fb.size(); ++i)
        pixels[i] = fb[i] ? 0xFFFFFFFF : 0x000000FF;   // white on / black off

    SDL_UpdateTexture(texture_, nullptr, pixels.data(),
                      static_cast<int>(DISPLAY_WIDTH * sizeof(std::uint32_t)));
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

} // namespace chip8
