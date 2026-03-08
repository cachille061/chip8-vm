#include "core/chip8.hpp"

#include <SDL.h>
#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: chip8 <rom>\n";
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    // Verify SDL works: create a window, show it briefly, then quit.
    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 320,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    std::cout << "CHIP-8 VM skeleton — SDL2 initialized successfully.\n";
    std::cout << "ROM path: " << argv[1] << "\n";

    SDL_Delay(1000);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
