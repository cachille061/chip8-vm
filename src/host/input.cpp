#include "input.hpp"

namespace chip8 {

namespace {

/// Map an SDL scancode to a CHIP-8 key index (0x0–0xF), or -1 if unmapped.
int scancode_to_key(SDL_Scancode sc)
{
    switch (sc) {
        case SDL_SCANCODE_1: return 0x1;
        case SDL_SCANCODE_2: return 0x2;
        case SDL_SCANCODE_3: return 0x3;
        case SDL_SCANCODE_4: return 0xC;
        case SDL_SCANCODE_Q: return 0x4;
        case SDL_SCANCODE_W: return 0x5;
        case SDL_SCANCODE_E: return 0x6;
        case SDL_SCANCODE_R: return 0xD;
        case SDL_SCANCODE_A: return 0x7;
        case SDL_SCANCODE_S: return 0x8;
        case SDL_SCANCODE_D: return 0x9;
        case SDL_SCANCODE_F: return 0xE;
        case SDL_SCANCODE_Z: return 0xA;
        case SDL_SCANCODE_X: return 0x0;
        case SDL_SCANCODE_C: return 0xB;
        case SDL_SCANCODE_V: return 0xF;
        default: return -1;
    }
}

} // anonymous namespace

bool Input::poll(std::array<bool, NUM_KEYS>& keypad)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_QUIT:
                return true;

            case SDL_KEYDOWN:
                if (ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    return true;
                if (int k = scancode_to_key(ev.key.keysym.scancode); k >= 0)
                    keypad[static_cast<std::size_t>(k)] = true;
                break;

            case SDL_KEYUP:
                if (int k = scancode_to_key(ev.key.keysym.scancode); k >= 0)
                    keypad[static_cast<std::size_t>(k)] = false;
                break;

            default:
                break;
        }
    }
    return false;
}

} // namespace chip8
